#include "AMA.h"

void Genetic::run(int maxIterNonProd, int timeLimit)
{	
	int nbIterNonProd = 1;
	for (int nbIter = 0 ; nbIterNonProd <= maxIterNonProd && clock()/CLOCKS_PER_SEC < timeLimit ; nbIter++)
	{	
		/* SELECTION AND CROSSOVER */
		crossoverOX(offspring, population->getBinaryTournament(),population->getBinaryTournament());

		/* LOCAL SEARCH */
		localSearch->run(offspring, params->penaltyCapacity, params->penaltyDuration, params->penaltyPack);
		bool isNewBest = population->addIndividual(offspring, true);//�ý�����һ�ε���addIndividual()�����Եڶ�������ΪTRUE����ʾ����������Կ�������Ϣ
		if (!offspring->isFeasible && std::rand()%2 == 0) // Repair half of the solutions in case of infeasibility
		{
			localSearch->run(offspring, params->penaltyCapacity*10., params->penaltyDuration*10., params->penaltyPack*10.);
			if (offspring->isFeasible)
				isNewBest = (population->addIndividual(offspring, false) || isNewBest);//�ý����ڶ��ε���addIndividual()�����Եڶ�������Ϊfalse
		}

		/* TRACKING THE NUMBER OF ITERATIONS SINCE LAST SOLUTION IMPROVEMENT */
		if (isNewBest) nbIterNonProd = 1;
		else nbIterNonProd ++ ;

		/* DIVERSIFICATION, PENALTY MANAGEMENT AND TRACES */
		if (nbIter % 100 == 0) population->managePenalties() ;//ÿ����100�Σ������³ͷ�ֵ��Ȩ��
		if (nbIter % 500 == 0) population->printState(nbIter, nbIterNonProd);//ÿ����500��

		/* FOR TESTS INVOLVING SUCCESSIVE RUNS UNTIL A TIME LIMIT: WE RESET THE ALGORITHM/POPULATION EACH TIME maxIterNonProd IS ATTAINED*/
		if (timeLimit != INT_MAX && nbIterNonProd == maxIterNonProd)
		{
			population->restart();
			nbIterNonProd = 1;
		}
	}
}

void Genetic::crossoverOX(Individual * result, const Individual * parent1, const Individual * parent2)
{
	// Frequency table to track the customers which have been already inserted
	std::vector <bool> freqClient = std::vector <bool> (params->nbClients + 1, false);

	// Picking the beginning and end of the crossover zone
	int start = std::rand() % params->nbClients;
	int end = std::rand() % params->nbClients;
	while (end == start)
		end = std::rand() % params->nbClients;

	// Copy in place the elements from start to end (possibly "wrapping around" the end of the array)
	int j = start;
	while (j % params->nbClients != (end + 1) % params->nbClients)
	{
		result->chromT[j % params->nbClients] = parent1->chromT[j % params->nbClients];
		freqClient[result->chromT[j % params->nbClients]] = true;
		j++;
	}

	// Fill the remaining elements in the order given by the second parent
	for (int i = 1; i <= params->nbClients; i++)
	{
		int temp = parent2->chromT[(end + i) % params->nbClients];
		if (freqClient[temp] == false)
		{
			result->chromT[j % params->nbClients] = temp;
			j++;
		}
	}

	// Completing the individual with the Split algorithm
	//split->generalSplit(result, parent1->myCostSol.nbRoutes);//����ԭ�еĴ���
	split->generalSplit(result, params->nbVehicles); //�����ҸĶ���Ĵ���
}

Genetic::Genetic(Params * params, Split * split, Population * population, LocalSearch * localSearch) : params(params), split(split), population(population), localSearch(localSearch)
{
	offspring = new Individual(params);//��̬�ڴ�����
}

Genetic::~Genetic(void)
{ 
	delete offspring ;//��ĳ�Ա�����к���һ��ָ��̬�ڴ�����ָ�룬���Ա���Ҫ�ڸ�������������н���delete
}
