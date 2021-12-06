#include "Split.h" 

void Split::generalSplit(Individual * indiv, int nbMaxVehicles)
{
	// Do not apply Split with fewer vehicles than the trivial (LP) bin packing bound
	maxVehicles = std::max<int>(nbMaxVehicles, std::ceil(params->totalDemand/params->vehicleCapacity));

	// Initialization of the data structures for the split algorithms
	for (int i = 1; i <= params->nbClients; i++)
	{   // cliSplit[i]对应的客户ID为indiv->chromT[i-1], 对应的具体对象为：params->cli[indiv->chromT[i-1]]
		cliSplit[i].demand = params->cli[indiv->chromT[i - 1]].demand;
		cliSplit[i].serviceTime = params->cli[indiv->chromT[i - 1]].serviceDuration;
		cliSplit[i].rects.clear(); //Fatal mistake!!! 致命的错误，如果不及时clear掉，每一次split之后，cliSplit[i]中的rects集合会持续累加！！！
		cliSplit[i].rects.insert(cliSplit[i].rects.end(), params->cli[indiv->chromT[i - 1]].rects.begin(), params->cli[indiv->chromT[i - 1]].rects.end());
		cliSplit[i].d0_x = params->timeCost[0][indiv->chromT[i - 1]];
		cliSplit[i].dx_0 = params->timeCost[indiv->chromT[i - 1]][0];
		if (i < params->nbClients)
			cliSplit[i].dnext = params->timeCost[indiv->chromT[i - 1]][indiv->chromT[i]];
		else cliSplit[i].dnext = -1.e30;  //即i为 giant tour上的最后一个结点时的情形
	}

	// We first try the simple split, and then the Split with limited fleet if this is not successful
	//生成当前解对象的chromR结构
	if (splitSimple(indiv) == 0)
		splitLF(indiv);

	// Build up the rest of the Individual structure
	//该函数会基于当前解对象中的chromR结构，来填充successors和predecessors结构，并计算出myCostSol结构
	indiv->evaluateCompleteCost();

}

int Split::splitSimple(Individual * indiv)
{
	// Reinitialize the potential structures
	potential[0][0] = 0;
	for (int i = 1; i <= params->nbClients; i++)
		potential[0][i] = 1.e30;
	for (int i = 0; i <= params->nbClients; i++)
		pred[0][i] = 0;

	// MAIN ALGORITHM -- Simple Split using Bellman's algorithm in topological order
	for (int i = 0; i < params->nbClients; i++)
	{
		double load = 0.;
		double distance = 0.;
		double serviceDuration = 0.;
		std::vector<rbp::RectSize> rects;
		for (int j = i + 1; j <= params->nbClients && load <= 2. * params->vehicleCapacity && params->bin.binPackingExcess(rects, params->binPackingExist) <=2; j++)
		{//每一个j结点，对应的是在有向无环图中，从i结点到本j结点的最短路径
			load += cliSplit[j].demand;
			serviceDuration += cliSplit[j].serviceTime;
			rects.insert(rects.end(), cliSplit[j].rects.begin(), cliSplit[j].rects.end());
			if (j == i + 1) distance += cliSplit[j].d0_x;
			else distance += cliSplit[j - 1].dnext;
			double cost = distance + cliSplit[j].dx_0
									+ params->penaltyCapacity * std::max<double>(load - params->vehicleCapacity, 0.)
									+ params->penaltyDuration * std::max<double>(distance + cliSplit[j].dx_0 + serviceDuration - params->durationLimit, 0.)
									+ params->penaltyPack * (params->bin.binPackingExcess(rects, params->binPackingExist));
			if (potential[0][i] + cost < potential[0][j])
			{
				potential[0][j] = potential[0][i] + cost;
				pred[0][j] = i;
			}
		}
	}
	//对于splitSimple算法而言，是不会抛出这个异常的
	if (potential[0][params->nbClients] > 1.e29) {
		throw std::string("Unlimited Fleet Split ERROR : no Split solution has been propagated until the last node  ");
	}


	// Filling the chromR structure
	for (int k = params->nbVehicles - 1; k >= maxVehicles; k--)
		indiv->chromR[k].clear();

	int end = params->nbClients;    //对应于拓扑序中的最后一个元素
	for (int k = maxVehicles - 1; k >= 0; k--)
	{   //当前路径对应于拓扑序中的：(i, j]，即 (begin, end]，对应的为indiv->chromT结构中的序列(indiv->chromT[begin-1], indiv->chromT[end-1]]
		indiv->chromR[k].clear();
		int begin = pred[0][end];
		for (int ii = begin; ii < end; ii++)
			indiv->chromR[k].push_back(indiv->chromT[ii]);
		end = begin;
	}

	// Return OK in case the Split algorithm reached the beginning of the routes
	//如果end最后为0，则说明在拓扑序中，从0到最后一个结点的弧的数量在maxVehicles以内，否则，则说明最短路径的弧的数量过多，超过了maxVehicles了
	return (end == 0);
}

// Split for problems with limited fleet
int Split::splitLF(Individual * indiv)
{
	// Initialize the potential structures
	potential[0][0] = 0;
	for (int k = 0; k <= maxVehicles; k++)
		for (int i = 1; i <= params->nbClients; i++)
			potential[k][i] = 1.e30;
	for (int k = 0; k <= maxVehicles; k++)
		for (int i = 0; i <= params->nbClients; i++)
			pred[k][i] = 0;

	// MAIN ALGORITHM -- Simple Split using Bellman's algorithm in topological order
	// This code has been maintained as it is very simple and can be easily adapted to a variety of constraints, whereas the O(n) Split has a more restricted application scope
	for (int k = 0; k < maxVehicles; k++)
	{
		for (int i = k; i < params->nbClients && potential[k][i] < 1.e29; i++)
		{
			double load = 0.;
			double serviceDuration = 0.;
			std::vector<rbp::RectSize> rects;
			double distance = 0.;
			for (int j = i + 1; j <= params->nbClients && load <= 2. * params->vehicleCapacity && params->bin.binPackingExcess(rects, params->binPackingExist)<=2; j++) // Setting a maximum limit on load infeasibility to accelerate the algorithm
			{
				load += cliSplit[j].demand;
				serviceDuration += cliSplit[j].serviceTime;
				rects.insert(rects.end(), cliSplit[j].rects.begin(), cliSplit[j].rects.end());
				if (j == i + 1) distance += cliSplit[j].d0_x;
				else distance += cliSplit[j - 1].dnext;
				double cost = distance + cliSplit[j].dx_0
										+ params->penaltyCapacity * std::max<double>(load - params->vehicleCapacity, 0.)
										+ params->penaltyDuration * std::max<double>(distance + cliSplit[j].dx_0 + serviceDuration - params->durationLimit, 0.)
										+ params->penaltyPack * (params->bin.binPackingExcess(rects, params->binPackingExist));
				if (potential[k][i] + cost < potential[k + 1][j])
				{
					potential[k + 1][j] = potential[k][i] + cost;
					pred[k + 1][j] = i;
				}
			}
		}
	}

	//有可能会抛出这个异常
	if (potential[maxVehicles][params->nbClients] > 1.e29) {
		throw std::string("Limited Fleet Split ERROR : no Split solution has been propagated until the last node  ");
	}

	// It could be cheaper to use a smaller number of vehicles
	double minCost = potential[maxVehicles][params->nbClients];
	int nbRoutes = maxVehicles;
	for (int k = 1; k < maxVehicles; k++)
		if (potential[k][params->nbClients] < minCost)
			{minCost = potential[k][params->nbClients]; nbRoutes = k;}

	// Filling the chromR structure: need index from 0 to nbRoutes-1, clear the remainder index from nbRoutes to params->nbVehicles-1
	for (int k = params->nbVehicles-1; k >= nbRoutes ; k--)
		indiv->chromR[k].clear();
	int end = params->nbClients;
	for (int k = nbRoutes - 1; k >= 0; k--)
	{
		indiv->chromR[k].clear();
		int begin = pred[k+1][end];
		for (int ii = begin; ii < end; ii++)
			indiv->chromR[k].push_back(indiv->chromT[ii]);
		end = begin;
	}

	// Return OK in case the Split algorithm reached the beginning of the routes
	return (end == 0);
}

Split::Split(Params * params): params(params)
{
	// Structures of the linear Split
	cliSplit = std::vector <ClientSplit>(params->nbClients + 1);
	potential = std::vector < std::vector <double> >(params->nbVehicles + 1, std::vector <double>(params->nbClients + 1, 1.e30));
	pred = std::vector < std::vector <int> >(params->nbVehicles + 1, std::vector <int>(params->nbClients + 1, 0));
}
