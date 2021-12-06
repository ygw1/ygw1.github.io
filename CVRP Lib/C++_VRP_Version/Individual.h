
#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H
#include "Params.h"
#include "MaxRectsBinPack.h"
class Individual;

struct CostSol
{
	double penalizedCost;		    // 即为当前解个体的总的适应度值, FitnessValue = distance + 对容量的惩罚值 + 对duration的惩罚值 + 装箱惩罚值
	int         nbRoutes;				    // Number of routes，计入的是非空的路线的数量
	double distance;			        // Total Distance
	double capacityExcess;		// Sum of excess load in all routes
	double durationExcess;		// Sum of excess duration in all routes
    double         packExcess;              // 在所有路线中，未能成功装箱的路线数量
    CostSol() { penalizedCost = 0.; nbRoutes = 0; distance = 0.; capacityExcess = 0.; durationExcess = 0.; packExcess = 0.; }
};

class Individual
{
public:

  Params * params ;													    // Problem parameters
  CostSol myCostSol;														// Solution cost parameters

  std::vector < int > chromT ;								// Giant tour representing the individual, do not include the deport Node. 规格: [params->nbClients]
                                                                                // chromT结构在Individual类的构造函数中就会产生
  std::vector < std::vector <int> > chromR ;		// For each vehicle, the associated sequence of deliveries (complete solution), 每个route中是不包含deport结点的. 规格：[params->nbVehicles] * [ ]
  
  std::vector < int > successors ;				// For each node, the successor in the solution (can be the depot 0)     规格： [params->nbClients + 1]
  std::vector < int > predecessors ;			// For each node, the predecessor in the solution (can be the depot 0) 规格：[params->nbClients + 1]
                                                  // 对于successors和predecessors的数据填充是在调用evaluateCompleteCost()函数时进行的，
                                                  // predecessors[i]和successors[i]对应的值即为ID为i的client在所在路线的前驱结点的client ID号和后继结点的client ID号
  
  std::multiset < std::pair < double, Individual* > > indivsPerProximity ;	// The other individuals in the population, ordered by increasing proximity (the set container follows a natural ordering based on the first value of the pair)
                                                                            //multiset是默认按照元素的key值从小到大进行排序
                                                                            // pair的结构是，第一个元素是它自身和别的individual的brokenPairsDistance，第二个元素是那个individual的指针。
  
  bool isFeasible;												// Feasibility status of the individual
  double biasedFitness;									// Biased fitness of the solution，需在在种群中才能被计算, biasedFitness=fitRank+(1 - nbElite/size())*divRank

  // Measuring cost of a solution from the information of chromR
  // 该函数会基于当前解对象中的chromR结构，来填充successors和predecessors结构，并计算出myCostSol结构
  void evaluateCompleteCost();

  // Removing an individual in the structure of proximity(n. 接近,邻近)
  //在Population源文件中被调用，当种群触发了筛选机制之后，就需要删除generation size个个体，而每次就会删除当前BaisedFitness值最差的个体，
  //当删除了这个最差的个体之后，需要对种群中余下的每个个体调用其removeProximity()函数，确保已被删除的个体不会出现在他们的indivsPerProximity中
  void removeProximity(Individual * indiv);

  // Distance measure with another individual
  double brokenPairsDistance(Individual * indiv2);

  // Returns the average distance of this individual with the nbClosest individuals
  double averageBrokenPairsDistanceClosest(int nbClosest) ;

  // Exports a solution in CVRPLib format (adds a final line with the computational time)
  //在Population文件中调用
  void exportCVRPLibFormat(std::string fileName);

  // Reads a solution in CVRPLib format, returns TRUE if the process worked, or FALSE if the file does not exist or is not readable
  //在Population文件中调用
  static bool readCVRPLibFormat(std::string fileName, std::vector<std::vector<int>> & readSolution, double & readCost);

  // Constructor: random individual,产生一个拥有随机性的chromT结构的个体
  Individual(Params * params);

  // Constructor: empty individual
  Individual();
};
#endif
