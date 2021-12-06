
#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <iostream>
#include <string>
#include <climits>

class CommandLine
{
public:
	std::string pathInstance;		// Instance path
	int seed = 0;			                // Random seed. Default value: 0
	bool allowFlip;                      // indicates whether the rectangles could be rotated.
	
	std::string pathSolution;		// Solution path
	int nbIter		 = 20000;		// Number of iterations without improvement until termination. Default value: 20,000 iterations
	int timeLimit   = INT_MAX;	// CPU time limit until termination in seconds. Default value: infinity
	std::string pathBKS = "";		// BKS path


	// Reads the line of command and extracts possible options
	CommandLine(int argc, char* argv[])
	{
		if (argc % 2 != 1 || argc > 13 || argc < 3)
		{
			std::cout << "----- NUMBER OF COMMANDLINE ARGUMENTS IS INCORRECT: " << argc << std::endl;
			display_help(); throw std::string("Incorrect line of command");
		}
		else
		{   //例如，可提供命令行参数："Instances\\CVRP\\X-n157-k13.vrp  Solution\\mySolution.sol -seed 6"
			pathInstance = std::string(argv[1]);     //必须提供的参数
			pathSolution = std::string(argv[2]);     //必须提供的参数
			// 可选提供的参数
			for (int i = 3; i < argc; i += 2)
			{
				if (std::string(argv[i]) == "-t")
					timeLimit = atoi(argv[i+1]);
				else if (std::string(argv[i]) == "-it")
					nbIter  = atoi(argv[i+1]);
				else if (std::string(argv[i]) == "-bks")
					pathBKS = std::string(argv[i+1]);
				else if (std::string(argv[i]) == "-seed")
					seed    = atoi(argv[i+1]);
				else
				{
					std::cout << "----- ARGUMENT NOT RECOGNIZED: " << std::string(argv[i]) << std::endl;
					display_help(); throw std::string("Incorrect line of command");
				}
			}
		}
	}

	//e.g. path: "Instances\\2LCVRP\\2l_cvrp3605.txt"
	CommandLine(std::string pathInstance, int seed, bool allowFlip):pathInstance(pathInstance), seed(seed), allowFlip(allowFlip)
	{
		//e.g. path: "Solution\\2LCVRP\\2l_cvrp3605_1_rotation.txt"
		pathSolution = pathInstance.assign(pathInstance.end() - 8, pathInstance.end() - 4);
		pathSolution = "Solution\\2LCVRP\\" + pathSolution;
		if (allowFlip)
			pathSolution += "_" + std::to_string(seed) + "_" + "rotation" + ".txt";
		else
			pathSolution += "_" + std::to_string(seed) + "_" + "oriented" + ".txt";
	}

	// Printing information about how to use the code
	void display_help()
	{
		std::cout << std::endl;
		std::cout << "-------------------------------------------------- HGS-CVRP algorithm (2020) --------------------------------------------------" << std::endl;
		std::cout << "Call with: ./genvrp instancePath solPath [-it nbIter] [-t myCPUtime] [-bks bksPath] [-seed mySeed] [-veh nbVehicles]           " << std::endl;
		std::cout << "[-it nbIterations] sets a maximum number of iterations without improvement. Defaults to 20,000                                 " << std::endl;
		std::cout << "[-t myCPUtime] sets a time limit in seconds. If this parameter is set the code will be run iteratively until the time limit    " << std::endl;
		std::cout << "[-bks bksPath] sets an optional path to a BKS. This file will be overwritten in case of improvement                            " << std::endl;
		std::cout << "[-seed mySeed] sets a fixed seed. Defaults to 0                                                                                " << std::endl;
		std::cout << "[-veh nbVehicles] sets a prescribed fleet size. Otherwise a reasonable UB on the the fleet size is calculated                  " << std::endl;
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << std::endl;
		std::cout << std::endl;
	}

};
#endif
