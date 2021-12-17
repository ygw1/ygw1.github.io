#include "group.h"

void Population::generatePopulation()
{
	for (int i = 0; i < 4*params->mu; i++)
	{
		bool splitResult = true;
		Individual* randomIndiv = new Individual(params);//将对象的内存开辟在stack内存里，但要记得释放掉
		split->generalSplit(randomIndiv, params->nbVehicles);

		localSearch->run(randomIndiv, params->penaltyCapacity, params->penaltyDuration, params->penaltyPack);
		//split->generalSplit(randomIndiv, params->nbVehicles);
		addIndividual(randomIndiv, true);  //第二个参数表明当前解个体是第一次调用此函数，计入其属性可行性信息
		if (!randomIndiv->isFeasible && std::rand() % 2 == 0)  // Repair half of the solutions in case of infeasibility
		{
			localSearch->run(randomIndiv, params->penaltyCapacity*10., params->penaltyDuration*10., params->penaltyPack*10.);
			if (randomIndiv->isFeasible) 
				addIndividual(randomIndiv, false); //第二个参数表明当前解个体是第二次调用此函数，其属性可行性信息不再计入
		} //在释放randomIndiv所指的内存空间之前，在addIndividual()函数内已经对所指对象进行了动态内存拷贝构造了
		delete randomIndiv;
	}
}

bool Population::addIndividual(const Individual * indiv, bool updateFeasible)
{
	//std::cout << "generate new individual! " << endl;
	if (updateFeasible)
	{
		listFeasibilityLoad.push_back(indiv->myCostSol.capacityExcess < MY_EPSILON);
		listFeasibilityDuration.push_back(indiv->myCostSol.durationExcess < MY_EPSILON);
		listFeasibilityPack.push_back(indiv->myCostSol.packExcess < MY_EPSILON);
		listFeasibilityLoad.pop_front();
		listFeasibilityDuration.pop_front();
		listFeasibilityPack.pop_front();

	}

	// Find the adequate subpopulation in relation to the individual feasibility
	SubPopulation & subpop = (indiv->isFeasible) ? feasibleSubpopulation : infeasibleSubpopulation;

	// Create a copy of the individual and updade the proximity structures calculating inter-individual distances
	Individual * myIndividual = new Individual(*indiv);  //动态内存分配+拷贝构造函数（浅拷贝足够了，因为Individual类结构中并没有动态分配的内存变量）

	for (Individual* myIndividual2 : subpop)
	{
		double myDistance = myIndividual->brokenPairsDistance(myIndividual2);
		myIndividual2->indivsPerProximity.insert({ myDistance, myIndividual });
		myIndividual->indivsPerProximity.insert({ myDistance, myIndividual2 });
	}

	// Identify the correct location in the population and insert the individual
	int place = (int)subpop.size();
	while (place > 0 && subpop[place - 1]->myCostSol.penalizedCost > indiv->myCostSol.penalizedCost - MY_EPSILON)
		place--;
	subpop.emplace(subpop.begin() + place, myIndividual); //类似于insert成员函数，按照penalizedCost值（=距离+各种惩罚值）来插入

	// Trigger a survivor selection if the maximimum population size is exceeded
	if ((int)subpop.size() > params->mu + params->lambda)
		while ((int)subpop.size() > params->mu)
			removeWorstBiasedFitness(subpop);//删除种群中较差的generation size个个体

	// Track best solution
	if (indiv->isFeasible && indiv->myCostSol.penalizedCost < bestSolutionRestart.myCostSol.penalizedCost - MY_EPSILON)
	{
		bestSolutionRestart = *indiv; //默认的对象赋值函数
		if (indiv->myCostSol.penalizedCost < bestSolutionOverall.myCostSol.penalizedCost - MY_EPSILON)
		{
			bestSolutionOverall = *indiv;
			searchProgress.push_back({ clock(),bestSolutionOverall.myCostSol.penalizedCost }); //记录每次产生的最好解的情况
		}
		return true;
	}
	else
		return false;
}

void Population::updateBiasedFitnesses(SubPopulation & pop)
{
	// Ranking the individuals based on their diversity contribution (decreasing order of distance)
	std::vector <std::pair <double, int> > ranking;
	for (int i = 0 ; i < (int)pop.size(); i++) 
		ranking.push_back({-pop[i]->averageBrokenPairsDistanceClosest(params->nbClose),i});//当前解同与它亲密的解的平均距离值越大，其多样性越好
	std::sort(ranking.begin(), ranking.end()); //按升序排，自动按照pair<first, second>结构中的first元素来升序排列

	// Updating the biased fitness values
	if (pop.size() == 1) 
		pop[0]->biasedFitness = 0;
	else
	{
		for (int i = 0; i < (int)pop.size(); i++)
		{   //按照ranking来遍历的，ranking.size()=pop.size()
			double divRank = (double)i / (double)(pop.size() - 1); // Ranking from 0 to 1
			double fitRank = (double)ranking[i].second / (double)(pop.size() - 1);
			if ((int)pop.size() <= params->nbElite) // Elite individuals cannot be bigger than population size
				pop[ranking[i].second]->biasedFitness = fitRank;
			else 
				pop[ranking[i].second]->biasedFitness = fitRank + (1.0 - (double)params->nbElite / (double)pop.size()) * divRank;
		}
	}
}

void Population::removeWorstBiasedFitness(SubPopulation & pop)
{
	updateBiasedFitnesses(pop);
	if (pop.size() <= 1) throw std::string("Eliminating the best individual: this should not occur in HGS");

	Individual * worstIndividual = NULL;
	int worstIndividualPosition = -1;
	bool isWorstIndividualClone = false;
	double worstIndividualBiasedFitness = -1.e30;
	for (int i = 1; i < (int)pop.size(); i++)
	{
		bool isClone = (pop[i]->averageBrokenPairsDistanceClosest(1) < MY_EPSILON); // A distance equal to 0 indicates that a clone exists
		if ((isClone && !isWorstIndividualClone) || (isClone == isWorstIndividualClone && pop[i]->biasedFitness > worstIndividualBiasedFitness))
		{
			worstIndividualBiasedFitness = pop[i]->biasedFitness;
			isWorstIndividualClone = isClone;
			worstIndividualPosition = i;
			worstIndividual = pop[i];
		}
	}

	pop.erase(pop.begin() + worstIndividualPosition); // Removing the individual from the population
	for (Individual* myIndividual2 : pop)
		myIndividual2->removeProximity(worstIndividual); // Cleaning its distances from the other individuals in the population
	delete worstIndividual; // Freeing memory
}

void Population::restart()
{
	std::cout << "----- RESET: CREATING A NEW POPULATION -----" << std::endl;
	//因为种群中的解个体都是new出来的，即在 Population::addIndividual(Individual*, bool)中通过对实参的“动态内存分配+拷贝构造”得到的，故都需要delete
	for (Individual* indiv : feasibleSubpopulation) 
		delete indiv;
	for (Individual* indiv : infeasibleSubpopulation) 
		delete indiv;
	feasibleSubpopulation.clear();
	infeasibleSubpopulation.clear();
	bestSolutionRestart = Individual();//bestSolutionRestart对象为静态内存分配的，不需要delete
	generatePopulation();
}

void Population::managePenalties()
{
	// Setting some bounds [0.1,1000] to the penalty values for safety
	double fractionFeasibleLoad = (double)std::count(listFeasibilityLoad.begin(), listFeasibilityLoad.end(), true) / (double)listFeasibilityLoad.size();
	if (fractionFeasibleLoad < params->targetFeasible - 0.05 && params->penaltyCapacity < 1000) params->penaltyCapacity = std::min<double>(params->penaltyCapacity * 1.2,1000.);
	else if (fractionFeasibleLoad > params->targetFeasible + 0.05 && params->penaltyCapacity > 0.1) params->penaltyCapacity = std::max<double>(params->penaltyCapacity * 0.85, 0.1);

	// Setting some bounds [0.1,1000] to the penalty values for safety
	double fractionFeasibleDuration = (double)std::count(listFeasibilityDuration.begin(), listFeasibilityDuration.end(), true) / (double)listFeasibilityDuration.size();
	if (fractionFeasibleDuration < params->targetFeasible - 0.05 && params->penaltyDuration < 1000)	params->penaltyDuration = std::min<double>(params->penaltyDuration * 1.2,1000.);
	else if (fractionFeasibleDuration > params->targetFeasible + 0.05 && params->penaltyDuration > 0.1) params->penaltyDuration = std::max<double>(params->penaltyDuration * 0.85, 0.1);

	// Setting some bounds [0.1,1000] to the penalty values for safety
	double fractionFeasiblePack = (double)std::count(listFeasibilityPack.begin(), listFeasibilityPack.end(), true) / (double)listFeasibilityPack.size();
	if (fractionFeasiblePack < params->targetFeasible - 0.05 && params->penaltyPack < 1000)	params->penaltyPack = std::min<double>(params->penaltyPack * 1.2, 1000.);
	else if (fractionFeasiblePack > params->targetFeasible + 0.05 && params->penaltyPack > 0.1) params->penaltyPack = std::max<double>(params->penaltyPack * 0.85, 0.1);

	// Update the evaluations
	//对非可行种群中所有个体的penalizedCost值进行更新，因为惩罚系数变了。
	for (int i = 0; i < (int)infeasibleSubpopulation.size(); i++)
		infeasibleSubpopulation[i]->myCostSol.penalizedCost = infeasibleSubpopulation[i]->myCostSol.distance
																										+ params->penaltyCapacity * infeasibleSubpopulation[i]->myCostSol.capacityExcess
																										+ params->penaltyDuration * infeasibleSubpopulation[i]->myCostSol.durationExcess
																										+ params->penaltyPack * infeasibleSubpopulation[i]->myCostSol.packExcess;


	// If needed, reorder the individuals in the infeasible subpopulation since the penalty values have changed (simple bubble sort for the sake of simplicity)
	//保持种群内的个体还是按照penalizedCost来进行升序排列的
	for (int i = 0; i < (int)infeasibleSubpopulation.size(); i++)
	{
		for (int j = 0; j < (int)infeasibleSubpopulation.size() - i - 1; j++)
		{
			if (infeasibleSubpopulation[j]->myCostSol.penalizedCost > infeasibleSubpopulation[j + 1]->myCostSol.penalizedCost + MY_EPSILON)
			{
				//只需将指向两个存储堆内存上的解个体的指针进行交换，高效
				Individual * indiv = infeasibleSubpopulation[j];
				infeasibleSubpopulation[j] = infeasibleSubpopulation[j + 1];
				infeasibleSubpopulation[j + 1] = indiv;
			}
		}
	}
}

Individual * Population::getBinaryTournament ()
{
	Individual * individual1 ;
	Individual * individual2 ;

	updateBiasedFitnesses(feasibleSubpopulation);
	updateBiasedFitnesses(infeasibleSubpopulation);
	
	int place1 = std::rand() % (feasibleSubpopulation.size() + infeasibleSubpopulation.size()) ;
	if (place1 >= (int)feasibleSubpopulation.size()) 
		individual1 = infeasibleSubpopulation[place1 - feasibleSubpopulation.size()] ;
	else 
		individual1 = feasibleSubpopulation[place1] ;

	int place2 = std::rand() % (feasibleSubpopulation.size() + infeasibleSubpopulation.size()) ;
	if (place2 >= (int)feasibleSubpopulation.size()) 
		individual2 = infeasibleSubpopulation[place2 - feasibleSubpopulation.size()] ;
	else 
		individual2 = feasibleSubpopulation[place2] ;

	if (individual1->biasedFitness < individual2->biasedFitness) 
		return individual1 ;
	else
		return individual2 ;		
}

Individual * Population::getBestFeasible ()
{
	if (!feasibleSubpopulation.empty())
		return feasibleSubpopulation[0] ;
	else
		return NULL ;
}

Individual * Population::getBestInfeasible ()
{
	if (!infeasibleSubpopulation.empty())
		return infeasibleSubpopulation[0] ;
	else
		return NULL ;
}

Individual * Population::getBestFound()
{
	if (bestSolutionOverall.myCostSol.penalizedCost < 1.e29)
		return &bestSolutionOverall;//取地址
	else
		return NULL;
}

void Population::printState(int nbIter, int nbIterNoImprovement)
{
	std::printf("It %6d %6d | T(s) %.2f", nbIter, nbIterNoImprovement, (double)clock() / (double)CLOCKS_PER_SEC);

	if (getBestFeasible() != NULL) std::printf(" | Feas %zu %.2f %.2f", feasibleSubpopulation.size(), getBestFeasible()->myCostSol.penalizedCost, getAverageCost(feasibleSubpopulation));
	else std::printf(" | NO-FEASIBLE");

	if (getBestInfeasible() != NULL) std::printf(" | Inf %zu %.2f %.2f", infeasibleSubpopulation.size(), getBestInfeasible()->myCostSol.penalizedCost, getAverageCost(infeasibleSubpopulation));
	else std::printf(" | NO-INFEASIBLE");

	std::printf(" | Div %.2f %.2f", getDiversity(feasibleSubpopulation), getDiversity(infeasibleSubpopulation));
	std::printf(" | Feas %.2f %.2f %.2f", (double)std::count(listFeasibilityLoad.begin(), listFeasibilityLoad.end(), true) / (double)listFeasibilityLoad.size(), (double)std::count(listFeasibilityDuration.begin(), listFeasibilityDuration.end(), true) / (double)listFeasibilityDuration.size(), (double)std::count(listFeasibilityPack.begin(), listFeasibilityPack.end(), true) / (double)listFeasibilityPack.size());
	std::printf(" | Pen %.2f %.2f %.2f", params->penaltyCapacity, params->penaltyDuration, params->penaltyPack);
	std::cout << std::endl;
}

double Population::getDiversity(const SubPopulation & pop)
{
	double average = 0.;
	int size = std::min<int>(params->mu, pop.size()); // Only monitoring the "mu" better solutions to avoid too much noise in the measurements
	for (int i = 0; i < size; i++) average += pop[i]->averageBrokenPairsDistanceClosest(size);
	if (size > 0) return average / (double)size;
	else return -1.0;
}

double Population::getAverageCost(const SubPopulation & pop)
{
	double average = 0.;
	int size = std::min<int>(params->mu, pop.size()); // Only monitoring the "mu" better solutions to avoid too much noise in the measurements
	for (int i = 0; i < size; i++) average += pop[i]->myCostSol.penalizedCost;
	if (size > 0) return average / (double)size;
	else return -1.0;
}

void Population::exportBKS(std::string fileName)
{
	double readCost;
	std::vector<std::vector<int>> readSolution;
	std::cout << "----- CHECKING FOR POSSIBLE BKS UPDATE" << std::endl;
	bool readOK = Individual::readCVRPLibFormat(fileName, readSolution, readCost);
	//将文件中的已知最好解同本次程序运行中的当前最好解bestSolutionOverall进行比较
	if (bestSolutionOverall.myCostSol.penalizedCost < 1.e29 && (!readOK || bestSolutionOverall.myCostSol.penalizedCost < readCost - MY_EPSILON))
	{
		std::cout << "----- NEW BKS: " << bestSolutionOverall.myCostSol.penalizedCost << " !!!" << std::endl;
		bestSolutionOverall.exportCVRPLibFormat(fileName); //覆盖掉原有的
	}
}

void Population::exportSearchProgress(std::string fileName, std::string instanceName, int seedRNG)
{
	std::ofstream myfile(fileName);
	for (std::pair<clock_t, double> state : searchProgress)
		myfile << instanceName << ";" << seedRNG << ";" << state.second << ";" << (double)state.first / (double)CLOCKS_PER_SEC << std::endl;
}

Population::Population(Params * params, Split * split, LocalSearch * localSearch) : params(params), split(split), localSearch(localSearch)
{
	//initialize the pool of listFeasibilityLoad and listFeasibilityDuration with all TRUE, then each time a new individual is generated, its feasibility concerning the load or duration can be 
	//added into the listFeasibilityLoad and listFeasibilityDuration as a tail element respectively, in the meanwhile, both the list need to pop its front element.
	listFeasibilityLoad = std::list<bool>(100, true);
	listFeasibilityDuration = std::list<bool>(100, true);
	listFeasibilityPack = std::list<bool>(100, true);
	generatePopulation();
}

Population::~Population()
{
	for (int i = 0; i < (int)feasibleSubpopulation.size(); i++) delete feasibleSubpopulation[i];
	for (int i = 0; i < (int)infeasibleSubpopulation.size(); i++) delete infeasibleSubpopulation[i];
}