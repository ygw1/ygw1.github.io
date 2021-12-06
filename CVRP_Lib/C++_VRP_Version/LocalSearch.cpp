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
		if (std::rand() % params->nbGranular == 0)  // Designed to use O(nbGranular * n) time overall to avoid possible bottlenecks//这里的O()为时间复杂度分析
			std::random_shuffle(params->correlatedVertices[i].begin(), params->correlatedVertices[i].end()); //执行这条语句的概率为5%，制造扰动.
	
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
			nodeU->whenLastTestedRI = nbMoves;  //打上时间戳
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
					//可设置断点Params::testBug1==44&&Params::testBug2==1007
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
			/*尝试着将非空route上的客户节点移到空route上，看是否能降低cost*/
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
					if (CircleSector::overlap(routeU->sector, routeV->sector))//判断两个route的扇区是否重叠
						swapStar();
			}
		}


	}

	// Register the solution produced by the LS in the individual
	//将经过localSearch改进后的完整的解结构给生成出来
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
	if (nodeUIndex == nodeYIndex) return false;//这是当V为depot结点时的状况

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
		//只是模拟更改，并未真正执行对route的更改， routeU->penalty是当前route的惩罚值
		costSuppV += penaltyExcessDuration(routeV->duration + costSuppV + serviceU)
			+ penaltyExcessLoad(routeV->load + loadU)
			+penaltyExcessPack(rectsRouteV)
			- routeV->penalty;
	}

	if (costSuppU + costSuppV > -MY_EPSILON) return false;
	//执行更改
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
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//加入nodeU的结点
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//加入nodeX的结点

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
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//加入nodeU的结点
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//加入nodeX的结点

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
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//加入nodeU的结点
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//加入nodeX的结点

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
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());//加入nodeV的结点
		rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeY->cour].rects.begin(), params->cli[nodeY->cour].rects.end());//加入nodeY的结点

		rectsRouteV.insert(rectsRouteV.end(), nodeV->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), nodeY->next->remainderRectsList.begin(), nodeY->next->remainderRectsList.end());
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());//加入nodeU的结点
		rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeX->cour].rects.begin(), params->cli[nodeX->cour].rects.end());//加入nodeX的结点

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

	//进行相关操作之后，这条route的成本变化.   delta=S_new - S_old, the move will execute only when the delta value is negative, which means that the move will lead to the reduce of the cost
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
		//routeU->penalty 只是routeU对超过的部分的惩罚值，不包含行驶距离.
	
	if (cost > -MY_EPSILON) return false;
	//原有路径的指向
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
	// LocalSearch::swapStar()基于当前的localSearch对象中的routeU和routeV两个指针来展开工作.
	SwapStarElement myBestSwapStar;

	// Preprocessing insertion costs
	preprocessInsertions(routeU, routeV);
	preprocessInsertions(routeV, routeU);

	// given the  definitized LocalSearch::routeU and LocalSearch::routeV,  find the optimal swap* execution for these two routes, and store the execution into the myBestSwapStar.
	for (nodeU = routeU->depot->next; !nodeU->isDepot; nodeU = nodeU->next)
	{
		for (nodeV = routeV->depot->next; !nodeV->isDepot; nodeV = nodeV->next)
		{   //注意，这个deltaPenRouteU其实不大严谨，但是如果在route的duration constraint不考虑的情况下，这是Okay的；其实加了duration constraint，也是Okay的
			//如果考虑装箱，deltaPenRouteU=penaltyExcessLoad(routeU)+penaltyExcessPack(routeU) - routeU->penalty
			double deltaPenRouteU = penaltyExcessLoad(routeU->load + params->cli[nodeV->cour].demand - params->cli[nodeU->cour].demand) - routeU->penalty;
			double deltaPenRouteV = penaltyExcessLoad(routeV->load + params->cli[nodeU->cour].demand - params->cli[nodeV->cour].demand) - routeV->penalty;

			std::vector<rbp::RectSize> rectsRouteU, rectsRouteV;
			rectsRouteU.insert(rectsRouteU.end(), nodeU->prev->cumulatedRectsList.begin(), nodeU->prev->cumulatedRectsList.end());
			rectsRouteU.insert(rectsRouteU.end(), nodeU->next->remainderRectsList.begin(), nodeU->next->remainderRectsList.end());
			rectsRouteU.insert(rectsRouteU.end(), params->cli[nodeV->cour].rects.begin(), params->cli[nodeV->cour].rects.end());

			rectsRouteV.insert(rectsRouteV.end(), nodeV->prev->cumulatedRectsList.begin(), nodeV->prev->cumulatedRectsList.end());
			rectsRouteV.insert(rectsRouteV.end(), nodeV->next->remainderRectsList.begin(), nodeV->next->remainderRectsList.end());
			rectsRouteV.insert(rectsRouteV.end(), params->cli[nodeU->cour].rects.begin(), params->cli[nodeU->cour].rects.end());
			// Quick filter: possibly early elimination of many SWAP* due to the capacity constraints/penalties and bounds 出界 on insertion costs
			//这里还可以加入装箱约束对应的惩罚值
			//这个if语句是为了减少SWAP*操作，对于当前而言，只知道要对routeU和routeV执行SWAP*操作，但不知道要将U插到routeV中的何处，也不知道要将V插入到routeU中的何处
			//在这种情况下，关于Load的变化可以算出来,当两条路线的capacity约束不会被违反才会进行操作
			if (deltaPenRouteU + routeU->penalty + deltaPenRouteV + routeV->penalty < 0. + MY_EPSILON)
			{
				SwapStarElement mySwapStar;
				mySwapStar.U = nodeU;
				mySwapStar.V = nodeV;

				// Evaluate best reinsertion cost of U in the route of V where V has been removed
				//这个函数是一个预评估，考虑在routeV上忽略V的存在，记为routeV'，然后计算将U插入到routeV'上的最佳的位置时，routeV'在distance上的变化
				//此时仅需要考虑距离的变化即可，因为不论将结点U插入到routeV'的何处，带来的在此路线上的penaltyExcessLoad,penaltyExcessDuration,penaltyExcessPack都是一样的
				double extraV = getCheapestInsertSimultRemoval(nodeU, nodeV, mySwapStar.bestPositionU);

				// Evaluate best reinsertion cost of V in the route of U where U has been removed
				double extraU = getCheapestInsertSimultRemoval(nodeV, nodeU, mySwapStar.bestPositionV);

				// Evaluating final moveCost, moveCost = delta(distance)+delta(penaltyExcessDuration)+ delta(penaltyExcessLoad)+ delta(penaltyExcessPack)
				// for a route: Duration = totalDistance + SumOfEachClientServiceDuration, so, delta(Duration)=delta(totalDistance)+delta(SumOfEachClientServiceDuration)
				mySwapStar.moveCost = nodeU->deltaRemoval + extraU        //delta(totalDistance) for routeU: 将U移走的变化+将U移走后再将V插入过来的变化
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
	//这个for循环其实不是标准的swap*操作，它是扩展的RELOCATE算子，就是将结点U插到它对应的routeV上的最佳位置；对结点V不进行任何操作
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
	//这个for循环是扩展的第二个RELOCATE算子，就是将结点V插到它对应的routeU上的最佳位置；对结点U不进行任何操作
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
	//myBestSwapStar存储着最好的操作，如果是标准swap *操作(即U插到routeV，同时V插入到routeU)，则bestPositionU和bestPositionV均为非空
	//若对应的是扩展的第一个RELOCATE算子，则bestPositionU非空，bestPositionV为空
	//若对应的是扩展的第二个RELOCATE算子，则bestPositionV非空，bestPositionU为空
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

	// Compute insertion in the place of V：将U插入到原来V所在的位置(考虑V结点已从所在的route中被抹去了之后的情形)
	double deltaCost = params->timeCost[V->prev->cour][U->cour] + params->timeCost[U->cour][V->next->cour] - params->timeCost[V->prev->cour][V->next->cour];
	if (!found || deltaCost < bestCost)
	{
		bestPosition = V->prev;//此时对应的是U和V位置互换
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
		if (R2->whenLastModified > bestInsertClient[R2->cour][U->cour].whenLastCalculated)//避免重复计算
		{
			bestInsertClient[R2->cour][U->cour].reset();//将U顾客节点在R2上的最佳插入的候选位置(有3个)进行重置.
			bestInsertClient[R2->cour][U->cour].whenLastCalculated = nbMoves;
			//首先把U插入到route R2的deport后面
			bestInsertClient[R2->cour][U->cour].bestCost[0] = params->timeCost[0][U->cour] + params->timeCost[U->cour][R2->depot->next->cour] - params->timeCost[0][R2->depot->next->cour];
			bestInsertClient[R2->cour][U->cour].bestLocation[0] = R2->depot;
			// 其次，依次去试探将U插在客户结点V身后的情况
			for (Node * V = R2->depot->next; !V->isDepot; V = V->next)
			{
				double deltaCost = params->timeCost[V->cour][U->cour] + params->timeCost[U->cour][V->next->cour] - params->timeCost[V->cour][V->next->cour];
				bestInsertClient[R2->cour][U->cour].compareAndAdd(deltaCost, V);
			}
		}
	}
}

//把U插到V身后
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

//交换U和V的位置
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

//这个函数用于对当前LocalSearch对象中存储的某条具体的route进行相关更新
//通过链表访问该route中的相关结点(结点存储在clients结构中)，然后更新每个结点的如下信息：
//position,  cumulatedLoad,  cumulatedTime,  cumulatedRectList,  cumulatedReversalDistance
//最后，更新该route的整体性信息(存储在routes结构中)，如下：
//duration  load  rectList  penalty  nbCustomers  reversalDistance  whenLastModified  polarAngleBarycenter
void LocalSearch::updateRouteData(Route * myRoute)
{
	Params::testBug3++;
	int myplace = 0;//路线中第一个customer的位置为1
	double myload = 0.;
	double mytime = 0.;
	std::vector<rbp::RectSize> myRectList;//正向累积
	std::vector<rbp::RectSize> myReverseRectList;//逆向累积
	double myReversalDistance = 0.; //这个参数表示，如果将这条路逆着走，那么同顺着走相比，产生的差异：distance(逆着走)-distance(顺着走)
	double cumulatedX = 0.;
	double cumulatedY = 0.;

	Node * mynode = myRoute->depot; //mynode 是一个哨兵，通过它实现依次访问
	//初始化哨兵结点
	mynode->position = 0;
	mynode->cumulatedLoad = 0.;
	mynode->cumulatedTime = 0.;
	mynode->cumulatedRectsList.clear(); 
	mynode->remainderRectsList.clear();
	mynode->cumulatedReversalDistance = 0.;

	bool firstIt = true;
	//顺序遍历：依次更新当前route中的每个Client Node以及尾哨兵结点的信息
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
			if (firstIt) myRoute->sector.initialize(params->cli[mynode->cour].polarAngle);//用第一个访问的client结点的极角来初始化该route的扇形区域sector.
			else myRoute->sector.extend(params->cli[mynode->cour].polarAngle);//随着新的client的加入，route的circle sector也在自动更新
		}
		firstIt = false;
	}

	//逆向遍历：依次更新当前route中的每个Node的remainderRectsList信息
	bool reverseTraversalstart = true;
	while (!mynode->isDepot || reverseTraversalstart) {
		mynode = mynode->prev;
		myReverseRectList.insert(myReverseRectList.end(), params->cli[mynode->cour].rects.begin(), params->cli[mynode->cour].rects.end());
		mynode->remainderRectsList = myReverseRectList;
		reverseTraversalstart = false;
	}


	//再更新整个Route的信息
	myRoute->duration = mytime;
	myRoute->load = myload;
	myRoute->rectsList = myRectList;
	myRoute->penalty = penaltyExcessDuration(mytime) + penaltyExcessLoad(myload) + penaltyExcessPack(myRectList);
	myRoute->nbCustomers = myplace-1;
	myRoute->reversalDistance = myReversalDistance;
	// Remember "when" this route has been last modified (will be used to filter unnecessary move evaluations)
	//每一次调用这个updateRouteData()函数时，一般就是对应的route结构发生变动了，即表明这个route的结构在Local Search中发生过变动,
	//通过这个变量来记录这个route最近一次改动对应的时间戳.
	myRoute->whenLastModified = nbMoves ;

	//下面的条件语句处理route是否为非空的情况。注意，在Set容器中，erase一个不存在的元素不会引发报错，inseart一个重复的元素的话，Set中也只会存一份.
	if (myRoute->nbCustomers == 0)
	{
		myRoute->polarAngleBarycenter = 1.e30; //空route的重心极角值设置为无穷大，便于后面chromR和chromT的重组(按照极角从小到大排)
		emptyRoutes.insert(myRoute->cour); //指示这个route为空,把它的ID号加入到Set容器中.
	}
	else
	{  //更新这个route的重心
		myRoute->polarAngleBarycenter = atan2(cumulatedY/(double)myRoute->nbCustomers - params->cli[0].coordY, cumulatedX/(double)myRoute->nbCustomers - params->cli[0].coordX);
		emptyRoutes.erase(myRoute->cour);  //set容器擦除一个不存在的数也不会产生bug
	}
}

void LocalSearch::loadIndividual(Individual * indiv)
{
	emptyRoutes.clear();
	nbMoves = 0; 
	// 将当前indiv的每一条route同LocalSearch环境中的结构进行对接
	// indiv->chromR[r]的头哨兵、尾哨兵、整体性信息，分别存储在LocalSearch对象中的depots[r]、depotsEnd[r]、routes[r]
	//注意，下标r从0开始标号，到nbVeh-1：0, 1, 2, ..., nbV-1
	for (int r = 0; r < params->nbVehicles; r++)
	{
		Node * myDepot = &depots[r];
		Node * myDepotFin = &depotsEnd[r];
		Route * myRoute = &routes[r];
		myDepot->prev = myDepotFin;//头哨兵
		myDepotFin->next = myDepot;//尾哨兵
		
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
			myClient->next = myDepotFin;//指向哨兵（tailer）
			myDepotFin->prev = myClient;//尾哨兵指向头哨兵
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
	std::vector < std::pair <double, int> > routePolarAngles ;// pair的结构：<route的重心, route的Index>
	for (int r = 0; r < params->nbVehicles; r++)
		routePolarAngles.push_back(std::pair <double, int>(routes[r].polarAngleBarycenter, r));
	std::sort(routePolarAngles.begin(), routePolarAngles.end()); // empty routes have a polar angle of 1.e30, and therefore will always appear at the end
	//将Local Search内部存储的解读取出来，按照质心排序，然后依次赋值给外部的indiv个体的chromR和chromeT.
	int pos = 0;
	for (int r = 0; r < params->nbVehicles; r++)
	{
		indiv->chromR[r].clear();
		Node * node = depots[routePolarAngles[r].second].next;//deports中的元素相当于哨兵，指向了其所对应的route的头结点(client node)
		while (!node->isDepot)
		{
			indiv->chromT[pos] = node->cour;   //T means tour, chromT is a giant tour without deport and delimiters，giant tour根据各个route的重心的弧度值由小到大排序，依次拼接
			indiv->chromR[r].push_back(node->cour);  //R means route, chromR is a complete solution, each element is a visiting route without deport.
			node = node->next;
			pos++;
		}
	}

	//更新indiv的chromR结构后，就可调用indiv的管家函数来更新自身的相关变量的信息.
	indiv->evaluateCompleteCost();
}

LocalSearch::LocalSearch(Params * params) : params (params)
{
	clients = std::vector < Node >(params->nbClients + 1);
	routes = std::vector < Route >(params->nbVehicles);
	depots = std::vector < Node >(params->nbVehicles);
	depotsEnd = std::vector < Node >(params->nbVehicles);
	// bestInsertClient的大小为nbVehicles*(nbClients+1), 元素bestInsertClient[Rid][ClientID]表示序号为ClientID的顾客结点要插入到第Rid条route中的最佳候选位置
	bestInsertClient = std::vector < std::vector <ThreeBestInsert> >(params->nbVehicles, std::vector <ThreeBestInsert>(params->nbClients + 1));

	for (int i = 0; i <= params->nbClients; i++) 
	{ 
		clients[i].cour = i; 
		clients[i].isDepot = false; //clients[0]对应的并不是deport，是一个哨兵头结点.
	}
	for (int i = 0; i < params->nbVehicles; i++)
	{
		routes[i].cour = i;
		routes[i].depot = &depots[i];//在route里设置它所对应的路线的头哨兵

		depots[i].cour = 0;
		depots[i].isDepot = true;
		depots[i].route = &routes[i];//头哨兵指向它对应的route

		depotsEnd[i].cour = 0;//设置尾哨兵
		depotsEnd[i].isDepot = true;
		depotsEnd[i].route = &routes[i];//尾哨兵指向它对应的route
	}
	for (int i = 1 ; i <= params->nbClients ; i++) orderNodes.push_back(i);
	for (int r = 0 ; r < params->nbVehicles ; r++) orderRoutes.push_back(r);
}

