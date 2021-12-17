
#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H
#include "Params.h"
#include "MaxRectsBinPack.h"
class Individual;

struct CostSol
{
	double penalizedCost;		    // ��Ϊ��ǰ�������ܵ���Ӧ��ֵ, FitnessValue = distance + �������ĳͷ�ֵ + ��duration�ĳͷ�ֵ + װ��ͷ�ֵ
	int         nbRoutes;				    // Number of routes��������Ƿǿյ�·�ߵ�����
	double distance;			        // Total Distance
	double capacityExcess;		// Sum of excess load in all routes
	double durationExcess;		// Sum of excess duration in all routes
    double         packExcess;              // ������·���У�δ�ܳɹ�װ���·������
    CostSol() { penalizedCost = 0.; nbRoutes = 0; distance = 0.; capacityExcess = 0.; durationExcess = 0.; packExcess = 0.; }
};

class Individual
{
public:

  Params * params ;													    // Problem parameters
  CostSol myCostSol;														// Solution cost parameters

  std::vector < int > chromT ;								// Giant tour representing the individual, do not include the deport Node. ���: [params->nbClients]
                                                                                // chromT�ṹ��Individual��Ĺ��캯���оͻ����
  std::vector < std::vector <int> > chromR ;		// For each vehicle, the associated sequence of deliveries (complete solution), ÿ��route���ǲ�����deport����. ���[params->nbVehicles] * [ ]
  
  std::vector < int > successors ;				// For each node, the successor in the solution (can be the depot 0)     ��� [params->nbClients + 1]
  std::vector < int > predecessors ;			// For each node, the predecessor in the solution (can be the depot 0) ���[params->nbClients + 1]
                                                  // ����successors��predecessors������������ڵ���evaluateCompleteCost()����ʱ���еģ�
                                                  // predecessors[i]��successors[i]��Ӧ��ֵ��ΪIDΪi��client������·�ߵ�ǰ������client ID�źͺ�̽���client ID��
  
  std::multiset < std::pair < double, Individual* > > indivsPerProximity ;	// The other individuals in the population, ordered by increasing proximity (the set container follows a natural ordering based on the first value of the pair)
                                                                            //multiset��Ĭ�ϰ���Ԫ�ص�keyֵ��С�����������
                                                                            // pair�Ľṹ�ǣ���һ��Ԫ����������ͱ��individual��brokenPairsDistance���ڶ���Ԫ�����Ǹ�individual��ָ�롣
  
  bool isFeasible;												// Feasibility status of the individual
  double biasedFitness;									// Biased fitness of the solution����������Ⱥ�в��ܱ�����, biasedFitness=fitRank+(1 - nbElite/size())*divRank

  // Measuring cost of a solution from the information of chromR
  // �ú�������ڵ�ǰ������е�chromR�ṹ�������successors��predecessors�ṹ���������myCostSol�ṹ
  void evaluateCompleteCost();

  // Removing an individual in the structure of proximity(n. �ӽ�,�ڽ�)
  //��PopulationԴ�ļ��б����ã�����Ⱥ������ɸѡ����֮�󣬾���Ҫɾ��generation size�����壬��ÿ�ξͻ�ɾ����ǰBaisedFitnessֵ���ĸ��壬
  //��ɾ����������ĸ���֮����Ҫ����Ⱥ�����µ�ÿ�����������removeProximity()������ȷ���ѱ�ɾ���ĸ��岻����������ǵ�indivsPerProximity��
  void removeProximity(Individual * indiv);

  // Distance measure with another individual
  double brokenPairsDistance(Individual * indiv2);

  // Returns the average distance of this individual with the nbClosest individuals
  double averageBrokenPairsDistanceClosest(int nbClosest) ;

  // Exports a solution in CVRPLib format (adds a final line with the computational time)
  //��Population�ļ��е���
  void exportCVRPLibFormat(std::string fileName);

  // Reads a solution in CVRPLib format, returns TRUE if the process worked, or FALSE if the file does not exist or is not readable
  //��Population�ļ��е���
  static bool readCVRPLibFormat(std::string fileName, std::vector<std::vector<int>> & readSolution, double & readCost);

  // Constructor: random individual,����һ��ӵ������Ե�chromT�ṹ�ĸ���
  Individual(Params * params);

  // Constructor: empty individual
  Individual();
};
#endif
