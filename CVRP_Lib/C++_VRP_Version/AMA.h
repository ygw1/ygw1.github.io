
#ifndef GENETIC_H
#define GENETIC_H

#include "Population.h"
#include "Individual.h"

class Genetic
{
private:

	Params * params;				// Problem parameters
	Split * split;					// Split algorithm
	Population * population;		// Population
	LocalSearch * localSearch;		// Local Search structure
	Individual * offspring;			// First individual to be used as input for the crossover

	// OX Crossover
	void crossoverOX(Individual * result, const Individual * parent1, const Individual * parent2);

public:

    // Running the genetic algorithm until maxIterNonProd consecutive iterations or a time limit
	// maxIterNonProd: maximum successive iteration without better solution produced.
	//在执行这个函数之前，初始种群已经形成了
    void run(int maxIterNonProd, int timeLimit) ;

	// Constructor
	Genetic(Params * params, Split * split, Population * population, LocalSearch * localSearch);

	// Destructor
	~Genetic(void);
};

#endif
