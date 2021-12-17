#include "LocalSearch.h" 

void LocalSearch::run(Individual * indiv, double penaltyCapacityLS, double penaltyDurationLS, double penaltyPackLS)
{
	this->penaltyCapacityLS = penaltyCapacityLS;
	this->penaltyDurationLS = penaltyDurationLS;
	this->penaltyPackLS = penaltyPackLS;
	loadIndividual(indiv);

	// Shuffling the order of the nodes explored by the LS to allow for more diversity in the search
	std::random_shuffle(orderNodes.begin(), orderNodes.end());
	std::random_shuffle(orderRoutes.begin(), orderRoutes.end());
	for (int i = 1; i <= params->nbClients; i++)
		if (std::rand() % params->nbGranular == 0)  // Designed to use O(nbGranular * n) time overall to avoid possible bottlenecks//�����O()Ϊʱ�临�Ӷȷ���
			std::random_shuffle(params->correlatedVertices[i].begin(), params->correlatedVertices[i].end()); //ִ���������ĸ���Ϊ5%�������Ŷ�.
	
	searchCompleted = false;
	for (loopID = 0; !searchCompleted; loopID++)
	{
		searchCompleted = true;
		Params::testBug1++;

		/* CLASSICAL ROUTE IMPROVEMENT (RI) MOVES SUBJECT TO A PROXIMITY RESTRICTION */
		for (int posU = 0; posU < params->nbClients; posU++)
		{
			//Params::testBug2++;
			//std::cout << "LOOP " << Params::testBug1 << "  RI Operation: " << Params::testBug2 << endl;
			nodeU = &clients[orderNodes[posU]];
			int lastTestRINodeU = nodeU->whenLastTestedRI;
			nodeU->whenLastTestedRI = nbMoves;  //����ʱ���
			for (int posV = 0; posV < (int)params->correlatedVertices[nodeU->cour].size(); posV++)
			{
				//Params::testBug2++;
				//cout << Params::testBug1<<"   "<<Params::testBug2 << endl;
				nodeV = &clients[params->correlatedVertices[nodeU->cour][posV]];
				if (loopID == 0 || std::max<int>(nodeU->route->whenLastModified, nodeV->route->whenLastModified) > lastTestRINodeU) // only evaluate moves involving routes that have been modified since last move evaluations for nodeU
				{
					// Randomizing the order of the neighborhoods within this loop does not matter much as we are already randomizing the order of the node pairs 
					// (and it's not very common to find improving moves of different types for the same node pair)
					
					setLocalVariablesRouteU();
					setLocalVariablesRouteV();
					//�����öϵ�Params::testBug1==44&&Params::testBug2==1007
					if (move1()) continue; // RELOCATE
					if (move2()) continue; // RELOCATE
					if (move3()) continue; // RELOCATE
					if (nodeUIndex <= nodeVIndex && move4()) continue; // SWAP
					if (move5()) continue; // SWAP
					if (nodeUIndex <= nodeVIndex && move6()) continue; // SWAP
					if (routeU == routeV && move7()) continue; // 2-OPT
					if (routeU != routeV && move8()) continue; // 2-OPT*
					if (routeU != routeV && move9()) continue; // 2-OPT*

					// Trying moves that insert nodeU directly after the depot
					if (nodeV->prev->isDepot)
					{
						nodeV = nodeV->prev;
						setLocalVariablesRouteV();
						if (move1()) continue; // RELOCATE
						if (move2()) continue; // RELOCATE
						if (move3()) continue; // RELOCATE
						if (routeU != routeV && move8()) continue; // 2-OPT*
						if (routeU != routeV && move9()) continue; // 2-OPT*
					}
				}
			}

			/* MOVES INVOLVING AN EMPTY ROUTE -- NOT TESTED IN THE FIRST LOOP TO AVOID INCREASING TOO MUCH THE FLEET SIZE */
			/*�����Ž��ǿ�route�ϵĿͻ��ڵ��Ƶ���route�ϣ����Ƿ��ܽ���cost*/
			if (loopID > 0 && !emptyRoutes.empty())
			{
				nodeV = routes[*emptyRoutes.begin()].depot;
				setLocalVariablesRouteU();
				setLocalVariablesRouteV();
				if (move1()) continue; // RELOCATE
				if (move2()) continue; // RELOCATE
				if (move3()) continue; // RELOCATE
				if (move9()) continue; // 2-OPT*
			}
		}

		/* (SWAP*) MOVES LIMITED TO ROUTE PAIRS WHOSE CIRCLE SECTORS OVERLAP */
		for (int rU = 0; rU < params->nbVehicles; rU++)
		{
			routeU = &routes[orderRoutes[rU]];
			int lastTestSWAPStarRouteU = routeU->whenLastTestedSWAPStar;
			routeU->whenLastTestedSWAPStar = nbMoves;
			for (int rV = 0; rV < params->nbVehicles; rV++)
			{
				//Params::testBug3++;
				//std::cout << "LOOP "<< Params::testBug1<<"  Swap* " << Params::testBug3 << endl;
				routeV = &routes[orderRoutes[rV]];
				if (routeU->nbCustomers > 0 && routeV->nbCustomers > 0 && routeU->cour < routeV->cour && (loopID == 0 || std::max<int>(routeU->whenLastModified, routeV->whenLastModified) > lastTestSWAPStarRouteU))
					if (CircleSector::overlap(routeU->sector, routeV->sector))//�ж�����route�������Ƿ��ص�
						swapStar();
			}
		}


	}

	// Register the solution produced by the LS in the individual
	//������localSearch�Ľ���������Ľ�ṹ�����ɳ���
	exportIndividual(indiv);
}

void LocalSearch::setLocalVariablesRouteU()
{
	routeU = nodeU->route;
	nodeX = nodeU->next;
	nodeUPrevIndex = nodeU->prev->cour;
	nodeUIndex = nodeU->cour;
	nodeXIndex = nodeX->cour;
	nodeXNextIndex = nodeX->next->cour;
	loadU    = params->cli[nodeUIndex].demand;
	serviceU = params->cli[nodeUIndex].serviceDuration;
	rectsU.clear();
	rectsU.insert(rectsU.end(), params->cli[nodeUIndex].rects.begin(), params->cli[nodeUIndex].rects.end());
	loadX	 = params->cli[nodeXIndex].demand;
	serviceX = params->cli[nodeXIndex].serviceDuration;
	rectsX.clear();
	rectsX.insert(rectsX.end(), params->cli[nodeXIndex].rects.begin(), params->cli[nodeXIndex].rects.end());
}

void LocalSearch::setLocalVariablesRouteV()
{
	routeV = nodeV->route;
	nodeY = nodeV->next;
	nodeYNextIndex = nodeY->next->cour;
	nodeVIndex = nodeV->cour;
	nodeVPrevIndex = nodeV->prev->cour;
	nodeYIndex = nodeY->cour;
	loadV    = params->cli[nodeVIndex].demand;
	serviceV = params->cli[nodeVIndex].serviceDuration;
	rectsV.clear();
	rectsV.insert(rectsV.end(), params->cli[nodeVIndex].rects.begin(), params->cli[nodeVIndex].rects.end());
	loadY	 = params->cli[nodeYIndex].demand;
	serviceY = params->cli[nodeYIndex].serviceDuration;
	rectsY.clear();
	rectsY.insert(rectsY.end(), params->cli[nodeYIndex].rects.begin(), params->cli[nodeYIndex].rects.end());
}

bool LocalSearch::move1()
{
	if (nodeUIndex == nodeYIndex) return false;//���ǵ�VΪdepot���ʱ��״��

	double costSuppU = params->timeCost[nodeUPrevIndex][nodeXIndex] - params->timeCost[nodeUPrevIndex][nodeUIndex] - params->timeCost[nodeUIndex][nodeXIndex];
	double costSuppV = params->timeCost[nodeVIndex][nodeUIndex] + params->timeCost[nodeUIndex][nodeYIndex] - params->timeCost[nodeVIndex][nodeYIndex];

	if (routeU != routeV)
	{
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeU->next->remainderRectsList.begin(), nodeU->next->remainderRectsList.end());

		rectsRouteV.insert(rectsRouteV.end(), routeV->rectsList.begin(), routeV->rectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());

		costSuppU += penaltyExcessDuration(routeU->duration + costSuppU - serviceU)
			+ penaltyExcessLoad(routeU->load - loadU)
			+ penaltyExcessPack(rectsRouteU)
			- routeU->penalty; 
		//ֻ��ģ����ģ���δ����ִ�ж�route�ĸ��ģ� routeU->penalty�ǵ�ǰroute�ĳͷ�ֵ
		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV + serviceU)
			+ penaltyExcessLoad(routeV->load + loadU)
			+penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;
	//ִ�и���
	insertNode(nodeU, nodeV);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (routeU != routeV) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move2()
{
	if (nodeU == nodeY || nodeV == nodeX || nodeX->isDepot) return false;

	double costSuppU = params->timeCost[nodeUPrevIndex][nodeXNextIndex] - params->timeCost[nodeUPrevIndex][nodeUIndex] - params->timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params->timeCost[nodeVIndex][nodeUIndex] + params->timeCost[nodeXIndex][nodeYIndex] - params->timeCost[nodeVIndex][nodeYIndex];

	if (routeU != routeV)
	{
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeX->next->remainderRectsList.begin(), nodeX->next->remainderRectsList.end());

		rectsRouteV.insert(rectsRouteV.end(), routeV->rectsList.begin(), routeV->rectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//����nodeU�Ľ��
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//����nodeX�Ľ��

		costSuppU += penaltyExcessDuration(routeU->duration + costSuppU - params->timeCost[nodeUIndex][nodeXIndex] - serviceU - serviceX)
			+ penaltyExcessLoad(routeU->load - loadU - loadX)
			+ penaltyExcessPack(rectsRouteU)
			- routeU->penalty;

		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV + params->timeCost[nodeUIndex][nodeXIndex] + serviceU + serviceX)
			+ penaltyExcessLoad(routeV->load + loadU + loadX)
			+ penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;

	insertNode(nodeU, nodeV);
	insertNode(nodeX, nodeU);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (routeU != routeV) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move3()
{
	if (nodeU == nodeY || nodeX == nodeV || nodeX->isDepot) return false;

	double costSuppU = params->timeCost[nodeUPrevIndex][nodeXNextIndex] - params->timeCost[nodeUPrevIndex][nodeUIndex] - params->timeCost[nodeUIndex][nodeXIndex] - params->timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params->timeCost[nodeVIndex][nodeXIndex] + params->timeCost[nodeXIndex][nodeUIndex] + params->timeCost[nodeUIndex][nodeYIndex] - params->timeCost[nodeVIndex][nodeYIndex];

	if (routeU != routeV)
	{
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeX->next->remainderRectsList.begin(), nodeX->next->remainderRectsList.end());

		rectsRouteV.insert(rectsRouteV.end(), routeV->rectsList.begin(), routeV->rectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//����nodeU�Ľ��
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//����nodeX�Ľ��

		costSuppU += penaltyExcessDuration(routeU->duration + costSuppU - serviceU - serviceX)
			+ penaltyExcessLoad(routeU->load - loadU - loadX)
			+ penaltyExcessPack(rectsRouteU)
			- routeU->penalty;

		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV + serviceU + serviceX)
			+ penaltyExcessLoad(routeV->load + loadU + loadX)
			+ penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;

	insertNode(nodeX, nodeV);
	insertNode(nodeU, nodeX);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (routeU != routeV) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move4()
{
	if (nodeUIndex == nodeVPrevIndex || nodeUIndex == nodeYIndex) return false;

	double costSuppU = params->timeCost[nodeUPrevIndex][nodeVIndex] + params->timeCost[nodeVIndex][nodeXIndex] - params->timeCost[nodeUPrevIndex][nodeUIndex] - params->timeCost[nodeUIndex][nodeXIndex];
	double costSuppV = params->timeCost[nodeVPrevIndex][nodeUIndex] + params->timeCost[nodeUIndex][nodeYIndex] - params->timeCost[nodeVPrevIndex][nodeVIndex] - params->timeCost[nodeVIndex][nodeYIndex];

	if (routeU != routeV)
	{
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeU->next->remainderRectsList.begin(), nodeU->next->remainderRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());

		rectsRouteV.insert(rectsRouteV.end(), nodeV ->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), nodeV ->next->remainderRectsList.begin(), nodeV->next->remainderRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());

		costSuppU += penaltyExcessDuration(routeU->duration + costSuppU + serviceV - serviceU)
			+ penaltyExcessLoad(routeU->load + loadV - loadU)
			+ penaltyExcessPack(rectsRouteU)
			- routeU->penalty;

		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV - serviceV + serviceU)
			+ penaltyExcessLoad(routeV->load + loadU - loadV)
			+ penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;

	swapNode(nodeU, nodeV);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (routeU != routeV) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move5()
{
	if (nodeU == nodeV->prev || nodeX == nodeV->prev || nodeU == nodeY || nodeX->isDepot) return false;

	double costSuppU = params->timeCost[nodeUPrevIndex][nodeVIndex] + params->timeCost[nodeVIndex][nodeXNextIndex] - params->timeCost[nodeUPrevIndex][nodeUIndex] - params->timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params->timeCost[nodeVPrevIndex][nodeUIndex] + params->timeCost[nodeXIndex][nodeYIndex] - params->timeCost[nodeVPrevIndex][nodeVIndex] - params->timeCost[nodeVIndex][nodeYIndex];

	if (routeU != routeV)
	{
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeX->next->remainderRectsList.begin(), nodeX->next->remainderRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());

		rectsRouteV.insert(rectsRouteV.end(), nodeV->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), nodeV->next->remainderRectsList.begin(), nodeV->next->remainderRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//����nodeU�Ľ��
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//����nodeX�Ľ��

		costSuppU += penaltyExcessDuration(routeU->duration + costSuppU - params->timeCost[nodeUIndex][nodeXIndex] + serviceV - serviceU - serviceX)
			+ penaltyExcessLoad(routeU->load + loadV - loadU - loadX)
			+ penaltyExcessPack(rectsRouteU)
			- routeU->penalty;

		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV + params->timeCost[nodeUIndex][nodeXIndex] - serviceV + serviceU + serviceX)
			+ penaltyExcessLoad(routeV->load + loadU + loadX - loadV)
			+ penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;

	swapNode(nodeU, nodeV);
	insertNode(nodeX, nodeU);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (routeU != routeV) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move6()
{
	if (nodeX->isDepot || nodeY->isDepot || nodeY == nodeU->prev || nodeU == nodeY || nodeX == nodeV || nodeV == nodeX->next) return false;

	double costSuppU = params->timeCost[nodeUPrevIndex][nodeVIndex] + params->timeCost[nodeYIndex][nodeXNextIndex] - params->timeCost[nodeUPrevIndex][nodeUIndex] - params->timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params->timeCost[nodeVPrevIndex][nodeUIndex] + params->timeCost[nodeXIndex][nodeYNextIndex] - params->timeCost[nodeVPrevIndex][nodeVIndex] - params->timeCost[nodeYIndex][nodeYNextIndex];

	if (routeU != routeV)
	{
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeX->next->remainderRectsList.begin(), nodeX->next->remainderRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());//����nodeV�Ľ��
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeY->cour].rects.begin(), params->cli[nodeY->cour].rects.end());//����nodeY�Ľ��

		rectsRouteV.insert(rectsRouteV.end(), nodeV->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), nodeY->next->remainderRectsList.begin(), nodeY->next->remainderRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//����nodeU�Ľ��
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//����nodeX�Ľ��

		costSuppU += penaltyExcessDuration(routeU->duration + costSuppU - params->timeCost[nodeUIndex][nodeXIndex] + params->timeCost[nodeVIndex][nodeYIndex] + serviceV + serviceY - serviceU - serviceX)
			+ penaltyExcessLoad(routeU->load + loadV + loadY - loadU - loadX)
			+ penaltyExcessPack(rectsRouteU)
			- routeU->penalty;

		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV + params->timeCost[nodeUIndex][nodeXIndex] - params->timeCost[nodeVIndex][nodeYIndex] - serviceV - serviceY + serviceU + serviceX)
			+ penaltyExcessLoad(routeV->load + loadU + loadX - loadV - loadY)
			+ penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;

	swapNode(nodeU, nodeV);
	swapNode(nodeX, nodeY);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (routeU != routeV) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move7()
{
	if (nodeU->position > nodeV->position|| nodeU->next == nodeV) return false;

	//������ز���֮������route�ĳɱ��仯.   delta=S_new - S_old, the move will execute only when the delta value is negative, which means that the move will lead to the reduce of the cost
	double cost = params->timeCost[nodeUIndex][nodeVIndex] + params->timeCost[nodeXIndex][nodeYIndex] - params->timeCost[nodeUIndex][nodeXIndex] - params->timeCost[nodeVIndex][nodeYIndex] + nodeV->cumulatedReversalDistance - nodeX->cumulatedReversalDistance;
	if (cost > -MY_EPSILON) return false;

	Node * nodeNum = nodeX->next;
	nodeX->prev = nodeNum;
	nodeX->next = nodeY;

	while (nodeNum != nodeV)
	{
		Node * temp = nodeNum->next;
		nodeNum->next = nodeNum->prev;
		nodeNum->prev = temp;
		nodeNum = temp;
	}

	nodeV->next = nodeV->prev;
	nodeV->prev = nodeU;
	nodeU->next = nodeV;
	nodeY->prev = nodeX;

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	return true;
}

bool LocalSearch::move8()
{
	std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
	rectsRouteU.insert(rectsRouteU.end(), nodeU->cumulatedRectsList.begin(), nodeU->cumulatedRectsList.end());
	rectsRouteU.insert(rectsRouteU.end(), nodeV->cumulatedRectsList.begin(), nodeV->cumulatedRectsList.end());

	rectsRouteV.insert(rectsRouteV.end(), nodeX->remainderRectsList.begin(), nodeX->remainderRectsList.end());
	rectsRouteV.insert(rectsRouteV.end(), nodeY->remainderRectsList.begin(), nodeY->remainderRectsList.end());

	//deltaCost=deltaDistance+deltaPenalty=((Cuv+Cxy-Cux-Cvy)+(V_cumulatedReversal+routeU_reversal - X_cumulatedReversal))   + ( totalPenaltyNew - totalPenaltyOld)
	double cost = params->timeCost[nodeUIndex][nodeVIndex] + params->timeCost[nodeXIndex][nodeYIndex] - params->timeCost[nodeUIndex][nodeXIndex] - params->timeCost[nodeVIndex][nodeYIndex]
		+ nodeV->cumulatedReversalDistance + routeU->reversalDistance - nodeX->cumulatedReversalDistance
		+ penaltyExcessPack(rectsRouteU)
		+ penaltyExcessPack(rectsRouteV)
		+ penaltyExcessDuration(nodeU->cumulatedTime + nodeV->cumulatedTime + nodeV->cumulatedReversalDistance + params->timeCost[nodeUIndex][nodeVIndex])
		+ penaltyExcessDuration(routeU->duration - nodeU->cumulatedTime - params->timeCost[nodeUIndex][nodeXIndex] + routeU->reversalDistance - nodeX->cumulatedReversalDistance + routeV->duration - nodeV->cumulatedTime - params->timeCost[nodeVIndex][nodeYIndex] + params->timeCost[nodeXIndex][nodeYIndex])
		+ penaltyExcessLoad(nodeU->cumulatedLoad + nodeV->cumulatedLoad)
		+ penaltyExcessLoad(routeU->load + routeV->load - nodeU->cumulatedLoad - nodeV->cumulatedLoad)
		- routeU->penalty - routeV->penalty;
		//routeU->penalty ֻ��routeU�Գ����Ĳ��ֵĳͷ�ֵ����������ʻ����.
	
	if (cost > -MY_EPSILON) return false;
	//ԭ��·����ָ��
	Node * depotU = routeU->depot;
	Node * depotV = routeV->depot;
	Node * depotUFin = routeU->depot->prev;
	Node * depotVFin = routeV->depot->prev;
	Node * depotVSuiv = depotV->next;

	Node * temp;
	Node * xx = nodeX;
	Node * vv = nodeV;

	while (!xx->isDepot)
	{
		temp = xx->next;
		xx->next = xx->prev;
		xx->prev = temp;
		xx->route = routeV;
		xx = temp;
	}

	while (!vv->isDepot)
	{
		temp = vv->prev;
		vv->prev = vv->next;
		vv->next = temp;
		vv->route = routeU;
		vv = temp;
	}

	if (!nodeV->isDepot && !nodeX->isDepot) {
		nodeU->next = nodeV;
		nodeV->prev = nodeU;
		nodeX->next = nodeY;
		nodeY->prev = nodeX;

		depotV->next = depotUFin->prev;
		depotV->next->prev = depotV;
		depotUFin->prev = depotVSuiv;
		depotUFin->prev->next = depotUFin;
	}
	else if (nodeV->isDepot && !nodeX->isDepot) {
		nodeX->next = nodeY;
		nodeY->prev = nodeX;

		depotV->next = depotUFin->prev;
		depotV->next->prev = depotV;
		depotV->prev = depotVFin;
		depotUFin->prev = nodeU;
		nodeU->next = depotUFin;
	}
	else if (!nodeV->isDepot && nodeX->isDepot) {
		nodeU->next = nodeV;
		nodeV->prev = nodeU;

		depotUFin->next = depotU;
		depotUFin->prev = depotVSuiv;
		depotUFin->prev->next = depotUFin;
		depotV->next = nodeY;
		nodeY->prev = depotV;
	}
	else if (nodeV->isDepot && nodeX->isDepot) {

		depotUFin->prev = nodeU;
		nodeU->next = depotUFin;
		depotV->next = nodeY;
		nodeY->prev = depotV;
	}

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	updateRouteData(routeV);
	return true;
}

bool LocalSearch::move9()
{
	std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
	rectsRouteU.insert(rectsRouteU.end(), nodeU->cumulatedRectsList.begin(), nodeU->cumulatedRectsList.end());
	rectsRouteU.insert(rectsRouteU.end(), nodeY->remainderRectsList.begin(), nodeY->remainderRectsList.end());

	rectsRouteV.insert(rectsRouteV.end(), nodeV->cumulatedRectsList.begin(), nodeV->cumulatedRectsList.end());
	rectsRouteV.insert(rectsRouteV.end(), nodeX->remainderRectsList.begin(), nodeX->remainderRectsList.end());

	double cost = params->timeCost[nodeUIndex][nodeYIndex] + params->timeCost[nodeVIndex][nodeXIndex] - params->timeCost[nodeUIndex][nodeXIndex] - params->timeCost[nodeVIndex][nodeYIndex]
		+ penaltyExcessDuration(nodeU->cumulatedTime + routeV->duration - nodeV->cumulatedTime - params->timeCost[nodeVIndex][nodeYIndex] + params->timeCost[nodeUIndex][nodeYIndex])
		+ penaltyExcessDuration(routeU->duration - nodeU->cumulatedTime - params->timeCost[nodeUIndex][nodeXIndex] + nodeV->cumulatedTime + params->timeCost[nodeVIndex][nodeXIndex])
		+ penaltyExcessPack(rectsRouteU)
		+ penaltyExcessPack(rectsRouteV)
		+ penaltyExcessLoad(nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad)
		+ penaltyExcessLoad(nodeV->cumulatedLoad + routeU->load - nodeU->cumulatedLoad)
		- routeU->penalty - routeV->penalty;

	if (cost > -MY_EPSILON) return false;

	Node * depotU = routeU->depot;
	Node * depotV = routeV->depot;
	Node * depotUFin = depotU->prev;
	Node * depotVFin = depotV->prev;
	Node * depotUpred = depotUFin->prev;

	Node * count = nodeY;
	while (!count->isDepot)
	{
		count->route = routeU;
		count = count->next;
	}

	count = nodeX;
	while (!count->isDepot)
	{
		count->route = routeV;
		count = count->next;
	}

	if (!nodeX->isDepot && !nodeY->isDepot) {
		nodeU->next = nodeY;
		nodeY->prev = nodeU;
		nodeV->next = nodeX;
		nodeX->prev = nodeV;

		depotUFin->prev = depotVFin->prev;
		depotUFin->prev->next = depotUFin;
		depotVFin->prev = depotUpred;
		depotVFin->prev->next = depotVFin;

	}
	else if (!nodeY->isDepot && nodeX->isDepot) {
		nodeU->next = nodeY;
		nodeY->prev = nodeU;

		depotUFin->prev = depotVFin->prev;
		depotVFin->prev->next = depotUFin;
		nodeV->next = depotVFin;
		depotVFin->prev = nodeV;
	}
	else if (nodeY->isDepot && !nodeX->isDepot) {
		nodeV->next = nodeX;
		nodeX->prev = nodeV;

		depotUpred->next = depotVFin;
		depotVFin->prev = depotUpred; 
		nodeU->next = depotUFin;
		depotUFin->prev = nodeU;

	}
	else if (nodeY->isDepot && nodeX->isDepot) {
		return false;
	}

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	updateRouteData(routeV);
	return true;
}

bool LocalSearch::swapStar()
{
	// LocalSearch::swapStar()���ڵ�ǰ��localSearch�����е�routeU��routeV����ָ����չ������.
	SwapStarElement myBestSwapStar;

	// Preprocessing insertion costs
	preprocessInsertions(routeU, routeV);
	preprocessInsertions(routeV, routeU);

	// given the  definitized LocalSearch::routeU and LocalSearch::routeV,  find the optimal swap* execution for these two routes, and store the execution into the myBestSwapStar.
	for (nodeU = routeU->depot->next; !nodeU->isDepot; nodeU = nodeU->next)
	{
		for (nodeV = routeV->depot->next; !nodeV->isDepot; nodeV = nodeV->next)
		{   //ע�⣬���deltaPenRouteU��ʵ�����Ͻ������������route��duration constraint�����ǵ�����£�����Okay�ģ���ʵ����duration constraint��Ҳ��Okay��
			//�������װ�䣬deltaPenRouteU=penaltyExcessLoad(routeU)+penaltyExcessPack(routeU) - routeU->penalty
			double deltaPenRouteU = penaltyExcessLoad(routeU->load + params->cli[nodeV->cour].demand - params->cli[nodeU->cour].demand) - routeU->penalty;
			double deltaPenRouteV = penaltyExcessLoad(routeV->load + params->cli[nodeU->cour].demand - params->cli[nodeV->cour].demand) - routeV->penalty;

			std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
			rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
			rectsRouteU.insert(rectsRouteU.end(), nodeU->next->remainderRectsList.begin(), nodeU->next->remainderRectsList.end());
			rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());

			rectsRouteV.insert(rectsRouteV.end(), nodeV->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
			rectsRouteV.insert(rectsRouteV.end(), nodeV->next->remainderRectsList.begin(), nodeV->next->remainderRectsList.end());
			rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());
			// Quick filter: possibly early elimination of many SWAP* due to the capacity constraints/penalties and bounds ���� on insertion costs
			//���ﻹ���Լ���װ��Լ����Ӧ�ĳͷ�ֵ
			//���if�����Ϊ�˼���SWAP*���������ڵ�ǰ���ԣ�ֻ֪��Ҫ��routeU��routeVִ��SWAP*����������֪��Ҫ��U�嵽routeV�еĺδ���Ҳ��֪��Ҫ��V���뵽routeU�еĺδ�
			//����������£�����Load�ı仯���������,������·�ߵ�capacityԼ�����ᱻΥ���Ż���в���
			if (deltaPenRouteU + routeU->penalty + deltaPenRouteV + routeV->penalty < 0. + MY_EPSILON)
			{
				SwapStarElement mySwapStar;
				mySwapStar.U = nodeU;
				mySwapStar.V = nodeV;

				// Evaluate best reinsertion cost of U in the route of V where V has been removed
				//���������һ��Ԥ������������routeV�Ϻ���V�Ĵ��ڣ���ΪrouteV'��Ȼ����㽫U���뵽routeV'�ϵ���ѵ�λ��ʱ��routeV'��distance�ϵı仯
				//��ʱ����Ҫ���Ǿ���ı仯���ɣ���Ϊ���۽����U���뵽routeV'�ĺδ����������ڴ�·���ϵ�penaltyExcessLoad,penaltyExcessDuration,penaltyExcessPack����һ����
				double extraV = getCheapestInsertSimultRemoval(nodeU, nodeV, mySwapStar.bestPositionU);

				// Evaluate best reinsertion cost of V in the route of U where U has been removed
				double extraU = getCheapestInsertSimultRemoval(nodeV, nodeU, mySwapStar.bestPositionV);

				// Evaluating final moveCost, moveCost = delta(distance)+delta(penaltyExcessDuration)+ delta(penaltyExcessLoad)+ delta(penaltyExcessPack)
				// for a route: Duration = totalDistance + SumOfEachClientServiceDuration, so, delta(Duration)=delta(totalDistance)+delta(SumOfEachClientServiceDuration)
				mySwapStar.moveCost = nodeU->deltaRemoval + extraU        //delta(totalDistance) for routeU: ��U���ߵı仯+��U���ߺ��ٽ�V��������ı仯
					                                        + nodeV->deltaRemoval + extraV       //delta(totalDistance) for routeV
					                                        + penaltyExcessDuration(routeU->duration + nodeU->deltaRemoval + extraU  - params->cli[nodeU->cour].serviceDuration + params->cli[nodeV->cour].serviceDuration)
					                                        + penaltyExcessDuration(routeV->duration + nodeV->deltaRemoval + extraV - params->cli[nodeV->cour].serviceDuration + params->cli[nodeU->cour].serviceDuration)
															+ penaltyExcessPack(rectsRouteU)
															+ penaltyExcessPack(rectsRouteV)
															+ deltaPenRouteU                                //deltaPenalty(routeU) =  penaltyExcessLoad(routeU) - routeU->penalty
															+ deltaPenRouteV;                               //deltaPenalty(routeV) =  penaltyExcessLoad(routeV) - routeV->penalty
				if (mySwapStar.moveCost < myBestSwapStar.moveCost)
					myBestSwapStar = mySwapStar;
			}
		}
	}

	// Including RELOCATE from nodeU towards routeV (costs nothing to include in the evaluation at this step since we already have the best insertion location)
	// Moreover, since the granularity criterion is different, this can lead to different improving moves
	//���forѭ����ʵ���Ǳ�׼��swap*������������չ��RELOCATE���ӣ����ǽ����U�嵽����Ӧ��routeV�ϵ����λ�ã��Խ��V�������κβ���
	for (nodeU = routeU->depot->next; !nodeU->isDepot; nodeU = nodeU->next)
	{
		SwapStarElement mySwapStar;
		mySwapStar.U = nodeU;
		mySwapStar.bestPositionU = bestInsertClient[routeV->cour][nodeU->cour].bestLocation[0];
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;

		rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), nodeU->next->remainderRectsList.begin(), nodeU->next->remainderRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), routeV->rectsList.begin(), routeV->rectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());

		double deltaDistRouteU = params->timeCost[nodeU->prev->cour][nodeU->next->cour] - params->timeCost[nodeU->prev->cour][nodeU->cour] - params->timeCost[nodeU->cour][nodeU->next->cour];
		double deltaDistRouteV = bestInsertClient[routeV->cour][nodeU->cour].bestCost[0];
		mySwapStar.moveCost = deltaDistRouteU + deltaDistRouteV              // delta(Distance)
													+ penaltyExcessLoad(routeU->load - params->cli[nodeU->cour].demand)
													+ penaltyExcessLoad(routeV->load + params->cli[nodeU->cour].demand)
													+ penaltyExcessDuration(routeU->duration + deltaDistRouteU - params->cli[nodeU->cour].serviceDuration)
													+ penaltyExcessDuration(routeV->duration + deltaDistRouteV + params->cli[nodeU->cour].serviceDuration)
													+ penaltyExcessPack(rectsRouteU)
													+ penaltyExcessPack(rectsRouteV)
													- routeU->penalty                                                                                     // delta(penalty_routeU)
													- routeV->penalty;                                                                                     // delta(penalty_routeV)

		if (mySwapStar.moveCost < myBestSwapStar.moveCost)
			myBestSwapStar = mySwapStar;
	}

	// Including RELOCATE from nodeV towards routeU
	//���forѭ������չ�ĵڶ���RELOCATE���ӣ����ǽ����V�嵽����Ӧ��routeU�ϵ����λ�ã��Խ��U�������κβ���
	for (nodeV = routeV->depot->next; !nodeV->isDepot; nodeV = nodeV->next)
	{
		SwapStarElement mySwapStar;
		mySwapStar.V = nodeV;
		mySwapStar.bestPositionV = bestInsertClient[routeU->cour][nodeV->cour].bestLocation[0];
		std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
		rectsRouteU.insert(rectsRouteU.end(), routeU->rectsList.begin(), routeU->rectsList.end());
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());
		rectsRouteV.insert(rectsRouteV.end(), nodeV->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), nodeV->next->remainderRectsList.begin(), nodeV->next->remainderRectsList.end());

		double deltaDistRouteU = bestInsertClient[routeU->cour][nodeV->cour].bestCost[0];
		double deltaDistRouteV = params->timeCost[nodeV->prev->cour][nodeV->next->cour] - params->timeCost[nodeV->prev->cour][nodeV->cour] - params->timeCost[nodeV->cour][nodeV->next->cour];
		mySwapStar.moveCost = deltaDistRouteU + deltaDistRouteV
													+ penaltyExcessLoad(routeU->load + params->cli[nodeV->cour].demand)
													+ penaltyExcessLoad(routeV->load - params->cli[nodeV->cour].demand)
													+ penaltyExcessDuration(routeU->duration + deltaDistRouteU + params->cli[nodeV->cour].serviceDuration)
													+ penaltyExcessDuration(routeV->duration + deltaDistRouteV - params->cli[nodeV->cour].serviceDuration)
													+ penaltyExcessPack(rectsRouteU)
													+ penaltyExcessPack(rectsRouteV)
													- routeU->penalty
													- routeV->penalty;

		if (mySwapStar.moveCost < myBestSwapStar.moveCost)
			myBestSwapStar = mySwapStar;
	}

	if (myBestSwapStar.moveCost > -MY_EPSILON) return false;

	// Applying the best move in case of improvement
	//myBestSwapStar�洢����õĲ���������Ǳ�׼swap *����(��U�嵽routeV��ͬʱV���뵽routeU)����bestPositionU��bestPositionV��Ϊ�ǿ�
	//����Ӧ������չ�ĵ�һ��RELOCATE���ӣ���bestPositionU�ǿգ�bestPositionVΪ��
	//����Ӧ������չ�ĵڶ���RELOCATE���ӣ���bestPositionV�ǿգ�bestPositionUΪ��
	if (myBestSwapStar.bestPositionU != NULL) insertNode(myBestSwapStar.U, myBestSwapStar.bestPositionU);
	if (myBestSwapStar.bestPositionV != NULL) insertNode(myBestSwapStar.V, myBestSwapStar.bestPositionV);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	updateRouteData(routeV);
	return true;
}

double LocalSearch::getCheapestInsertSimultRemoval(Node * U, Node * V, Node*& bestPosition)
{
	ThreeBestInsert * myBestInsert = &bestInsertClient[V->route->cour][U->cour];
	bool found = false;

	// Find best insertion in the route such that V is not next or pred (the candidate positions only belong to the top three locations)
	bestPosition = myBestInsert->bestLocation[0];
	double bestCost = myBestInsert->bestCost[0];
	found = (bestPosition != V && bestPosition->next != V);
	if (!found && myBestInsert->bestLocation[1] != NULL)
	{
		bestPosition = myBestInsert->bestLocation[1];
		bestCost = myBestInsert->bestCost[1];
		found = (bestPosition != V && bestPosition->next != V);
		if (!found && myBestInsert->bestLocation[2] != NULL)
		{
			bestPosition = myBestInsert->bestLocation[2];
			bestCost = myBestInsert->bestCost[2];
			found = true;
		}
	}

	// Compute insertion in the place of V����U���뵽ԭ��V���ڵ�λ��(����V����Ѵ����ڵ�route�б�Ĩȥ��֮�������)
	double deltaCost = params->timeCost[V->prev->cour][U->cour] + params->timeCost[U->cour][V->next->cour] - params->timeCost[V->prev->cour][V->next->cour];
	if (!found || deltaCost < bestCost)
	{
		bestPosition = V->prev;//��ʱ��Ӧ����U��Vλ�û���
		bestCost = deltaCost;
	}

	return bestCost;
}

void LocalSearch::preprocessInsertions(Route * R1, Route * R2)
{
	for (Node * U = R1->depot->next; !U->isDepot; U = U->next)
	{
		// Performs the preprocessing
		U->deltaRemoval = params->timeCost[U->prev->cour][U->next->cour] - params->timeCost[U->prev->cour][U->cour] - params->timeCost[U->cour][U->next->cour];
		if (R2->whenLastModified > bestInsertClient[R2->cour][U->cour].whenLastCalculated)//�����ظ�����
		{
			bestInsertClient[R2->cour][U->cour].reset();//��U�˿ͽڵ���R2�ϵ���Ѳ���ĺ�ѡλ��(��3��)��������.
			bestInsertClient[R2->cour][U->cour].whenLastCalculated = nbMoves;
			//���Ȱ�U���뵽route R2��deport����
			bestInsertClient[R2->cour][U->cour].bestCost[0] = params->timeCost[0][U->cour] + params->timeCost[U->cour][R2->depot->next->cour] - params->timeCost[0][R2->depot->next->cour];
			bestInsertClient[R2->cour][U->cour].bestLocation[0] = R2->depot;
			// ��Σ�����ȥ��̽��U���ڿͻ����V�������
			for (Node * V = R2->depot->next; !V->isDepot; V = V->next)
			{
				double deltaCost = params->timeCost[V->cour][U->cour] + params->timeCost[U->cour][V->next->cour] - params->timeCost[V->cour][V->next->cour];
				bestInsertClient[R2->cour][U->cour].compareAndAdd(deltaCost, V);
			}
		}
	}
}

//��U�嵽V���
void LocalSearch::insertNode(Node * U, Node * V)
{
	U->prev->next = U->next;
	U->next->prev = U->prev;
	V->next->prev = U;
	U->prev = V;
	U->next = V->next;
	V->next = U;
	U->route = V->route;
}

//����U��V��λ��
void LocalSearch::swapNode(Node * U, Node * V)
{
	Node * myVPred = V->prev;
	Node * myVSuiv = V->next;
	Node * myUPred = U->prev;
	Node * myUSuiv = U->next;
	Route * myRouteU = U->route;
	Route * myRouteV = V->route;

	myUPred->next = V;
	myUSuiv->prev = V;
	myVPred->next = U;
	myVSuiv->prev = U;

	U->prev = myVPred;
	U->next = myVSuiv;
	V->prev = myUPred;
	V->next = myUSuiv;

	U->route = myRouteV;
	V->route = myRouteU;
}

//����������ڶԵ�ǰLocalSearch�����д洢��ĳ�������route������ظ���
//ͨ��������ʸ�route�е���ؽ��(���洢��clients�ṹ��)��Ȼ�����ÿ������������Ϣ��
//position,  cumulatedLoad,  cumulatedTime,  cumulatedRectList,  cumulatedReversalDistance
//��󣬸��¸�route����������Ϣ(�洢��routes�ṹ��)�����£�
//duration  load  rectList  penalty  nbCustomers  reversalDistance  whenLastModified  polarAngleBarycenter
void LocalSearch::updateRouteData(Route * myRoute)
{
	Params::testBug3++;
	int myplace = 0;//·���е�һ��customer��λ��Ϊ1
	double myload = 0.;
	double mytime = 0.;
	std::vector<rbp::RectSize> myRectList;//�����ۻ�
	std::vector<rbp::RectSize> myReverseRectList;//�����ۻ�
	double myReversalDistance = 0.; //���������ʾ�����������·�����ߣ���ôͬ˳������ȣ������Ĳ��죺distance(������)-distance(˳����)
	double cumulatedX = 0.;
	double cumulatedY = 0.;

	Node * mynode = myRoute->depot; //mynode ��һ���ڱ���ͨ����ʵ�����η���
	//��ʼ���ڱ����
	mynode->position = 0;
	mynode->cumulatedLoad = 0.;
	mynode->cumulatedTime = 0.;
	mynode->cumulatedRectsList.clear(); 
	mynode->remainderRectsList.clear();
	mynode->cumulatedReversalDistance = 0.;

	bool firstIt = true;
	//˳����������θ��µ�ǰroute�е�ÿ��Client Node�Լ�β�ڱ�������Ϣ
	while (!mynode->isDepot || firstIt)
	{
		mynode = mynode->next;
		myplace++;
		mynode->position = myplace;
		myload += params->cli[mynode->cour].demand;
		mytime += params->timeCost[mynode->prev->cour][mynode->cour] + params->cli[mynode->cour].serviceDuration;
		myRectList.insert(myRectList.end(), params->cli[mynode->cour].rects.begin(), params->cli[mynode->cour].rects.end());
		myReversalDistance += params->timeCost[mynode->cour][mynode->prev->cour] - params->timeCost[mynode->prev->cour][mynode->cour] ;
		
		mynode->cumulatedLoad = myload;
		mynode->cumulatedTime = mytime;
		mynode->cumulatedRectsList = myRectList;
		mynode->cumulatedReversalDistance = myReversalDistance;

		if (!mynode->isDepot)
		{
			cumulatedX += params->cli[mynode->cour].coordX;
			cumulatedY += params->cli[mynode->cour].coordY;
			if (firstIt) myRoute->sector.initialize(params->cli[mynode->cour].polarAngle);//�õ�һ�����ʵ�client���ļ�������ʼ����route����������sector.
			else myRoute->sector.extend(params->cli[mynode->cour].polarAngle);//�����µ�client�ļ��룬route��circle sectorҲ���Զ�����
		}
		firstIt = false;
	}

	//������������θ��µ�ǰroute�е�ÿ��Node��remainderRectsList��Ϣ
	bool reverseTraversalstart = true;
	while (!mynode->isDepot || reverseTraversalstart) {
		mynode = mynode->prev;
		myReverseRectList.insert(myReverseRectList.end(), params->cli[mynode->cour].rects.begin(), params->cli[mynode->cour].rects.end());
		mynode->remainderRectsList = myReverseRectList;
		reverseTraversalstart = false;
	}


	//�ٸ�������Route����Ϣ
	myRoute->duration = mytime;
	myRoute->load = myload;
	myRoute->rectsList = myRectList;
	myRoute->penalty = penaltyExcessDuration(mytime) + penaltyExcessLoad(myload) + penaltyExcessPack(myRectList);
	myRoute->nbCustomers = myplace-1;
	myRoute->reversalDistance = myReversalDistance;
	// Remember "when" this route has been last modified (will be used to filter unnecessary move evaluations)
	//ÿһ�ε������updateRouteData()����ʱ��һ����Ƕ�Ӧ��route�ṹ�����䶯�ˣ����������route�Ľṹ��Local Search�з������䶯,
	//ͨ�������������¼���route���һ�θĶ���Ӧ��ʱ���.
	myRoute->whenLastModified = nbMoves ;

	//�����������䴦��route�Ƿ�Ϊ�ǿյ������ע�⣬��Set�����У�eraseһ�������ڵ�Ԫ�ز�����������inseartһ���ظ���Ԫ�صĻ���Set��Ҳֻ���һ��.
	if (myRoute->nbCustomers == 0)
	{
		myRoute->polarAngleBarycenter = 1.e30; //��route�����ļ���ֵ����Ϊ����󣬱��ں���chromR��chromT������(���ռ��Ǵ�С������)
		emptyRoutes.insert(myRoute->cour); //ָʾ���routeΪ��,������ID�ż��뵽Set������.
	}
	else
	{  //�������route������
		myRoute->polarAngleBarycenter = atan2(cumulatedY/(double)myRoute->nbCustomers - params->cli[0].coordY, cumulatedX/(double)myRoute->nbCustomers - params->cli[0].coordX);
		emptyRoutes.erase(myRoute->cour);  //set��������һ�������ڵ���Ҳ�������bug
	}
}

void LocalSearch::loadIndividual(Individual * indiv)
{
	emptyRoutes.clear();
	nbMoves = 0; 
	// ����ǰindiv��ÿһ��routeͬLocalSearch�����еĽṹ���жԽ�
	// indiv->chromR[r]��ͷ�ڱ���β�ڱ�����������Ϣ���ֱ�洢��LocalSearch�����е�depots[r]��depotsEnd[r]��routes[r]
	//ע�⣬�±�r��0��ʼ��ţ���nbVeh-1��0, 1, 2, ..., nbV-1
	for (int r = 0; r < params->nbVehicles; r++)
	{
		Node * myDepot = &depots[r];
		Node * myDepotFin = &depotsEnd[r];
		Route * myRoute = &routes[r];
		myDepot->prev = myDepotFin;//ͷ�ڱ�
		myDepotFin->next = myDepot;//β�ڱ�
		
		if (!indiv->chromR[r].empty())
		{
			Node * myClient = &clients[indiv->chromR[r][0]];
			myClient->route = myRoute;
			myClient->prev = myDepot;
			myDepot->next = myClient;
			for (int i = 1; i < (int)indiv->chromR[r].size(); i++)
			{
				Node * myClientPred = myClient;
				myClient = &clients[indiv->chromR[r][i]]; 
				myClient->prev = myClientPred;
				myClientPred->next = myClient;
				myClient->route = myRoute;
			}
			myClient->next = myDepotFin;//ָ���ڱ���tailer��
			myDepotFin->prev = myClient;//β�ڱ�ָ��ͷ�ڱ�
		}
		else
		{
			myDepot->next = myDepotFin;
			myDepotFin->prev = myDepot;
		}
		updateRouteData(&routes[r]);
		routes[r].whenLastTestedSWAPStar = -1;
		for (int i = 1; i <= params->nbClients; i++) // Initializing memory structures
			bestInsertClient[r][i].whenLastCalculated = -1;
	}

	for (int i = 1; i <= params->nbClients; i++) // Initializing memory structures
		clients[i].whenLastTestedRI = -1;
}

void LocalSearch::exportIndividual(Individual * indiv)
{
	std::vector < std::pair <double, int> > routePolarAngles ;// pair�Ľṹ��<route������, route��Index>
	for (int r = 0; r < params->nbVehicles; r++)
		routePolarAngles.push_back(std::pair <double, int>(routes[r].polarAngleBarycenter, r));
	std::sort(routePolarAngles.begin(), routePolarAngles.end()); // empty routes have a polar angle of 1.e30, and therefore will always appear at the end
	//��Local Search�ڲ��洢�Ľ��ȡ������������������Ȼ�����θ�ֵ���ⲿ��indiv�����chromR��chromeT.
	int pos = 0;
	for (int r = 0; r < params->nbVehicles; r++)
	{
		indiv->chromR[r].clear();
		Node * node = depots[routePolarAngles[r].second].next;//deports�е�Ԫ���൱���ڱ���ָ����������Ӧ��route��ͷ���(client node)
		while (!node->isDepot)
		{
			indiv->chromT[pos] = node->cour;   //T means tour, chromT is a giant tour without deport and delimiters��giant tour���ݸ���route�����ĵĻ���ֵ��С������������ƴ��
			indiv->chromR[r].push_back(node->cour);  //R means route, chromR is a complete solution, each element is a visiting route without deport.
			node = node->next;
			pos++;
		}
	}

	//����indiv��chromR�ṹ�󣬾Ϳɵ���indiv�ĹܼҺ����������������ر�������Ϣ.
	indiv->evaluateCompleteCost();
}

LocalSearch::LocalSearch(Params * params) : params (params)
{
	clients = std::vector < Node >(params->nbClients + 1);
	routes = std::vector < Route >(params->nbVehicles);
	depots = std::vector < Node >(params->nbVehicles);
	depotsEnd = std::vector < Node >(params->nbVehicles);
	// bestInsertClient�Ĵ�СΪnbVehicles*(nbClients+1), Ԫ��bestInsertClient[Rid][ClientID]��ʾ���ΪClientID�Ĺ˿ͽ��Ҫ���뵽��Rid��route�е���Ѻ�ѡλ��
	bestInsertClient = std::vector < std::vector <ThreeBestInsert> >(params->nbVehicles, std::vector <ThreeBestInsert>(params->nbClients + 1));

	for (int i = 0; i <= params->nbClients; i++) 
	{ 
		clients[i].cour = i; 
		clients[i].isDepot = false; //clients[0]��Ӧ�Ĳ�����deport����һ���ڱ�ͷ���.
	}
	for (int i = 0; i < params->nbVehicles; i++)
	{
		routes[i].cour = i;
		routes[i].depot = &depots[i];//��route������������Ӧ��·�ߵ�ͷ�ڱ�

		depots[i].cour = 0;
		depots[i].isDepot = true;
		depots[i].route = &routes[i];//ͷ�ڱ�ָ������Ӧ��route

		depotsEnd[i].cour = 0;//����β�ڱ�
		depotsEnd[i].isDepot = true;
		depotsEnd[i].route = &routes[i];//β�ڱ�ָ������Ӧ��route
	}
	for (int i = 1 ; i <= params->nbClients ; i++) orderNodes.push_back(i);
	for (int r = 0 ; r < params->nbVehicles ; r++) orderRoutes.push_back(r);
}

