#include "Individual.h" 

void Individual::evaluateCompleteCost()
{
	myCostSol = CostSol();
	for (int r = 0; r < params->nbVehicles; r++)
	{
		if (!chromR[r].empty())
		{
			double distance = params->timeCost[0][chromR[r][0]];//deport��·��r�ĵ�һ��client (ID��ΪchromR[r][0]) �ľ���
			double load = params->cli[chromR[r][0]].demand;
			double service = params->cli[chromR[r][0]].serviceDuration;
			std::vector<rbp::RectSize> rects = params->cli[chromR[r][0]].rects;
			predecessors[chromR[r][0]] = 0;  //��r���·�ߵĵ�һ��client��ǰ�����Ϊdeport.
			for (int i = 1; i < (int)chromR[r].size(); i++)
			{
				distance += params->timeCost[chromR[r][i-1]][chromR[r][i]];
				load += params->cli[chromR[r][i]].demand;
				service += params->cli[chromR[r][i]].serviceDuration;
				rects.insert(rects.end(), params->cli[chromR[r][i]].rects.begin(), params->cli[chromR[r][i]].rects.end());
				predecessors[chromR[r][i]] = chromR[r][i-1]; //update ��ǰ������е�predecessors�ṹ
				successors[chromR[r][i-1]] = chromR[r][i];      //update ��ǰ������е�successors�ṹ
			}
			successors[chromR[r][chromR[r].size()-1]] = 0;  //��r���·�ߵ����һ��client( IDΪchromR[r][chromR[r].size()-1] )�ĺ�̱��Ϊdeport.
			distance += params->timeCost[chromR[r][chromR[r].size()-1]][0];//����·��r�����һ��client��deport�ľ���.
			myCostSol.distance += distance;
			myCostSol.nbRoutes++; //ֻ�Ʒǿ�route������
			if (load > params->vehicleCapacity) myCostSol.capacityExcess += load - params->vehicleCapacity;
			if (distance + service > params->durationLimit) myCostSol.durationExcess += distance + service - params->durationLimit;
			myCostSol.packExcess += params->bin.binPackingExcess(rects, params->binPackingExist);
		}
	}

	myCostSol.penalizedCost = myCostSol.distance 
													+ myCostSol.capacityExcess * params->penaltyCapacity 
													+ myCostSol.durationExcess * params->penaltyDuration 
													+ myCostSol.packExcess * params->penaltyPack;
	isFeasible = (myCostSol.capacityExcess < MY_EPSILON && myCostSol.durationExcess < MY_EPSILON && myCostSol.packExcess < MY_EPSILON);
}

void Individual::removeProximity(Individual * indiv)
{
	auto it = indivsPerProximity.begin();
	while (it->second != indiv) ++it;
	indivsPerProximity.erase(it);
}

double Individual::brokenPairsDistance(Individual * indiv2)
{
	int differences = 0;
	for (int j = 1; j <= params->nbClients; j++)
	{
		if (successors[j] != indiv2->successors[j] && successors[j] != indiv2->predecessors[j]) differences++;
		if (predecessors[j] == 0 && indiv2->predecessors[j] != 0 && indiv2->successors[j] != 0) differences++;
	}
	return (double)differences/(double)params->nbClients;
}

double Individual::averageBrokenPairsDistanceClosest(int nbClosest) 
{
	double result = 0 ;
	int maxSize = std::min<int>(nbClosest, indivsPerProximity.size());
	auto it = indivsPerProximity.begin();//iterator, just like a pointer.
	for (int i=0 ; i < maxSize; i++)
	{
		result += it->first ;
		++it ;
	}
	return result/(double)maxSize ;
}

void Individual::exportCVRPLibFormat(std::string fileName)
{
	std::cout << "----- WRITING SOLUTION WITH VALUE " << myCostSol.penalizedCost << " IN : " << fileName << std::endl;
	std::ofstream myfile(fileName);
	if (myfile.is_open())
	{
		for (int k = 0; k < params->nbVehicles; k++)
		{
			if (!chromR[k].empty())
			{
				myfile << "Route #" << k+1 << ":"; // Route IDs start at 1 in the file format
				for (int i : chromR[k]) myfile << " " << i;
				myfile << std::endl;//д��"\n"��Ϊ���з�
			}
		}
		myfile << "Cost " << myCostSol.penalizedCost << std::endl;
		myfile << "Time " << (double)clock()/(double)CLOCKS_PER_SEC << std::endl;
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
}

bool Individual::readCVRPLibFormat(std::string fileName, std::vector<std::vector<int>> & readSolution, double & readCost)
{
	readSolution.clear();
	std::ifstream inputFile(fileName);
	if (inputFile.is_open())
	{
		std::string inputString;
		inputFile >> inputString; //����һ�е��ո��ַ���"Route"����inputString
		// Loops as long as the first line keyword is "Route"
		for (int r = 0; inputString == "Route" ; r++) 
		{
			readSolution.push_back(std::vector<int>());
			inputFile >> inputString; //�� "#1:" ������ɨ������get��ָ��ָ�����Ĺ˿�ID
			getline(inputFile, inputString);//getline������һ��������������
			std::stringstream ss(inputString);
			int inputCustomer;
			while (ss >> inputCustomer)   // Loops as long as there is an integer to read
				readSolution[r].push_back(inputCustomer);
			inputFile >> inputString;//get��ָ��ָ������һ�е� "Route"
		}
		if (inputString == "Cost")
		{
			inputFile >> readCost;
			return true; //��ǰ���أ�û�ж�ȡ��һ�е�Time��Ϣ
		}
		else std::cout << "----- UNEXPECTED WORD IN SOLUTION FORMAT: " << inputString << std::endl;
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
	return false;
}

Individual::Individual(Params * params) : params(params)
{
	successors = std::vector <int>(params->nbClients + 1);
	predecessors = std::vector <int>(params->nbClients + 1);
	chromR = std::vector < std::vector <int> >(params->nbVehicles);//R means route.
	chromT = std::vector <int>(params->nbClients);//T means tour, do not include the deport Node.
	for (int i = 0; i < params->nbClients; i++) chromT[i] = i + 1;
	std::random_shuffle(chromT.begin(), chromT.end());//���������һ��giant tour�����У��ԣ�1,2,...,nbClients) ��������ϴ��
	                                                                                             //�ú����ǻ���rand()�����������������������������Եģ�rand()��������srand ()�趨
}

Individual::Individual()
{
	myCostSol.penalizedCost = 1.e30;
}