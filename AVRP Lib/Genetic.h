/*  ---------------------------------------------------------------------- //
    Copyright (C) Yangguang Wang

	ygw@mail.ustc.edu.cn

	University of Science and Technology of China

//  ---------------------------------------------------------------------- */

#ifndef GENETIC_H
#define GENETIC_H

#include "Population.h"
#include "Params.h"
#include "Individu.h"
#include "time.h"
#include <stdlib.h>
#include <stdio.h> 
#include <vector>
#include <list>
#include <math.h>
using namespace std ;

class Genetic
{

private:

	// number of iterations without improvement (during the execution of the HGA)
	int nbIterNonProd ;

	// number of iterations (during the execution of the HGA)
	int nbIter ;

public:

	// allowed time
    clock_t ticks ;

	// printing search traces or not
	bool traces ;

	// pointer towards the population
	Population * population ;

	// working individuals (for the local search and crossover)
	// to work on some solutions we first create a copy in this kind of individuals
	// because the individual used for storage in the population do not contain all search data structures
	Individu * rejeton ; 
	Individu * rejeton2 ;
	Individu * rejetonP1 ;
	Individu * rejetonP2 ;
	Individu * rejetonBestFound ;
	Individu * rejetonBestFoundAll ;

	// Pointer towards the parameters of the problem
	Params * params ;

    // Running the algorithm until "maxIterations" total iterations have been reached, 
	// or "maxIterNonProd" consecutive iterations without improvement have been reached
	// nbRec is a parameter that says if we are in the main loop of the algorithm, or inside a decomposition phase
	void evolve (int maxIterNonProd, int nbRec) ;

	// sub functions to differenciate the behavior with a GA and with an ILS.
    void evolveHGA (int maxIterNonProd, int nbRec) ;
	void evolveILS () ;    

	// Repairing an infeasible solution
	void reparer ();

	// OX Crossover
	void crossOX ();

	// PIX Crossover
	void crossPIX ();

	// temporary structures used in the crossover
	vector < int > freqClient ;

	// regular management of the penalty coefficients
	void gererPenalites ();

	// Constructor
	Genetic(Params * params,Population * population, clock_t ticks, bool traces);

	// Destructor
	~Genetic(void);
};

#endif
