#include "AMA.h"
#include "inputLine.h"
#include "LocalSearch.h"
#include "bellman_ford.h"
#include <new>
using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		// Reading the arguments of the program
		//CommandLine commandline(argc, argv);
		CommandLine commandline("Instances\\2LCVRP\\2l_cvrp0601.txt", 1, false);

		// Reading the data file and initializing some data structures
		std::cout << "----- READING DATA SET: " << commandline.pathInstance << std::endl;

		//Params params(commandline.pathInstance, commandline.seed);
		Params params(&commandline);
		params.allowFlip ? std::cout << "----- TWO-DIMENSIONAL PACKING: ROTATED" << std::endl : std::cout << "----- TWO-DIMENSIONAL PACKING: ORIENTED" << std::endl;

		// Creating the Split and local search structures
		Split split(&params);
		LocalSearch localSearch(&params);

		// Initial population
		std::cout << "----- INSTANCE LOADED WITH " << params.nbClients << " CLIENTS AND " << params.nbVehicles << " VEHICLES" << std::endl;
		std::cout << "----- BUILDING INITIAL POPULATION" << std::endl;
		Population population(&params, &split, &localSearch);//���캯����������Ӧ�ĳ�ʼ��Ⱥ

		// Genetic algorithm
		std::cout << "----- STARTING GENETIC ALGORITHM" << std::endl;
		Genetic solver(&params, &split, &population, &localSearch);
		solver.run(commandline.nbIter, commandline.timeLimit);
		std::cout << "----- GENETIC ALGORITHM FINISHED, TIME SPENT: " << (double)clock()/(double)CLOCKS_PER_SEC << std::endl;

		// Exporting the best solution
		if (population.getBestFound() != NULL)
		{   //������γ����������ҵ�����ý�
			population.getBestFound()->exportCVRPLibFormat(commandline.pathSolution);
			//����ڱ��γ������е�ȫ�����У�ÿ����ÿ��н��ֵ����Ӧ�ĸ���ʱ��
			population.exportSearchProgress(commandline.pathSolution + ".PG.csv", commandline.pathInstance, commandline.seed);

			if (commandline.pathBKS != "")
				population.exportBKS(commandline.pathBKS);
		}
	}
	catch (const string& e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception& e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }
	return 0;
}
