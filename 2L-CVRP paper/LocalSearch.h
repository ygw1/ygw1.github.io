#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Individual.h"
#include "MaxRectsBinPack.h"

struct Node ;

// Structure containing a route
struct Route
{
	int cour;							                // Route index, ��0��ʼ��, �� params->nbVehicles-1
	int nbCustomers;					        // Number of customers visited in the route
	int whenLastModified;				    // "When" this route has been last modified
	int whenLastTestedSWAPStar;   // "When" the SWAP* moves for this route have been last tested
	Node * depot;						        // Pointer to the associated depot, ��route��ͷ�ڱ����(�ڱ���㲻�����ݣ�ֻ����������)

	double duration;					       // Total time spent on the route
	double load;						           // Total load on the route
	std::vector<rbp::RectSize> rectsList;  // Total rectangles on the route

	double reversalDistance;		     // Difference of cost if the route is reversed.
														 // ���������ʾ�����������·�����ߣ���ôͬ˳������ȣ������Ĳ��죺distance(������)-distance(˳����)
	double penalty;						     // Current sum of load, duration, and bin packing penalties�����ֻ�ǳ����Ĳ��ֵĳͷ�ֵ����������ʻ����.
														 // myRoute->penalty = penaltyExcessDuration(mytime) + penaltyExcessLoad(myload) + penaltyExcessPack(rectList)
	double polarAngleBarycenter;// Polar angle of the barycenter(����) of the route���õ��ǻ��ȱ�ʾ��.
	CircleSector sector;				     // Circle sector associated to the set of customers
};

struct Node
{
	bool isDepot;						// Tells whether this node represents a depot or not
	int cour;							    // Node index
	int position;						// Position in the route, the position of deport Node is 0.
	int whenLastTestedRI;		// "When" the RI moves for this node have been last tested, designed for RI.
	Node * next;						// Next node in the route order
	Node * prev;						// Previous node in the route order
	Route * route;					// Pointer towards the associated route
	
	double cumulatedLoad;				// Cumulated load on this route until the customer (including itself)
	double cumulatedTime;				// Cumulated time spent on this route until the customer (including itself)
	double cumulatedReversalDistance;	// Difference of cost if the segment of route [0,1,...,cour] is reversed (useful for 2-opt moves with asymmetric problems�ǶԳ�����)
																	// ���������ʾ������ӵ�ǰ��������߻س����㣬��ôͬ˳�Ŵӳ������ߵ��ˣ������Ĳ���: distance(cour,...,1,0) - distance(0,1,...,cour)
	std::vector<rbp::RectSize> cumulatedRectsList;  // Cumulated rectangles on this route from the first customer to this customer (including itself)

	std::vector<rbp::RectSize> remainderRectsList;  // Cumulated rectangles on this route from this customer to the final customer (including itself)

	double deltaRemoval;		// Difference of distance in the current route if the node is removed (used in SWAP*)��deltaRemoval = distance_new - distance_old
												// ע�⣺��������Ǽ��㽫��ǰnode������route�������������������ľ���ı仯ֵ��ע�⣺ֻ�Ǿ���ֵ�ı仯��
	                                            // ÿ������deltaRemoval����LocalSearch::preprocessInsertions() �������Լ���ģ�which is useful for swap*.
};

// Structure used in SWAP* to remember the three best insertion positions of a customer in a given route
struct ThreeBestInsert
{
	int whenLastCalculated;
	double bestCost[3];           //bestCost[i]ָ���ǽ������뵽bestLocation[i]��ָ�Ľ�����󣬱������route��distance�ϵĵı仯
	Node * bestLocation[3];   //bestLocation[i]ָ���Ǻ�ѡ�Ĳ���λ��ΪbestLocation[i]��ָ�Ľ������

	//����ǰNode���뵽placeInsert������֮�󣬶�Ӧ��distance�仯ΪcostInsert��Ȼ��
	//�����ֱ仯ͬ��Node��ǰ��������Ѻ�ѡλ�ý��б�, ������Ѻ�ѡ����λ��
	void compareAndAdd(double costInsert, Node * placeInsert)
	{
		if (costInsert >= bestCost[2]) return;
		else if (costInsert >= bestCost[1])
		{
			bestCost[2] = costInsert; bestLocation[2] = placeInsert;
		}
		else if (costInsert >= bestCost[0])
		{
			bestCost[2] = bestCost[1]; bestLocation[2] = bestLocation[1];
			bestCost[1] = costInsert; bestLocation[1] = placeInsert;
		}
		else
		{
			bestCost[2] = bestCost[1]; bestLocation[2] = bestLocation[1];
			bestCost[1] = bestCost[0]; bestLocation[1] = bestLocation[0];
			bestCost[0] = costInsert; bestLocation[0] = placeInsert;
		}
	}

	// Resets the structure (no insertion calculated)
	void reset()
	{
		bestCost[0] = 1.e30; bestLocation[0] = NULL;
		bestCost[1] = 1.e30; bestLocation[1] = NULL;
		bestCost[2] = 1.e30; bestLocation[2] = NULL;
	}

	ThreeBestInsert() { reset(); };
};

// Structured used to keep track of the best SWAP* move
struct SwapStarElement
{
	//moveCost��ʾ��U��V�ֱ���뵽bestPositionU��bestPositionV��󣬴���������·�߸�����(totalDistance + �������Եĳͷ�ֵ )��ǰ��仯ֵ
	//moveCost =  delta(distance_routeU) + delta(excessLoadPenalty_routeU) + delta(excessDurationPenalty_routeU) + delta(excessPackPenalty_routeU)
	//                     +delta(distance_routeV) + delta(excessLoadPenalty_routeV) + delta(excessDurationPenalty_routeV) + delta(excessPackPenalty_routeV)
	double moveCost = 1.e30 ; 
	Node * U = NULL ;
	Node * bestPositionU = NULL;
	Node * V = NULL;
	Node * bestPositionV = NULL;
};

// Main local search structure
class LocalSearch
{

private:
	
	Params * params ;							    // Problem parameters
	bool searchCompleted;						// Tells whether all moves have been evaluated without success
	int nbMoves;						// Total number of moves (RI and SWAP*) applied during the local search. 
												// ÿ��localSearch����ʹ��loadIndividual()����һ���µ�indiv���д����ʱ��, loadIndividual()�����ὫnbMoves��Ϊ0.
												// Attention: this is not only a simple counter, it is also used to avoid repeating move evaluations.
	std::vector < int > orderNodes;			// Randomized order for checking the nodes in the RI local search������1,2,3,...,nbClients ��һ������.
	std::vector < int > orderRoutes;			// Randomized order for checking the routes in the SWAP* local search������0,1,2,3,...,(nbVehicles-1) ��һ������.
	std::set < int > emptyRoutes;				// indices of all empty routes
	int loopID;									// Current loop index

	/* THE SOLUTION IS REPRESENTED AS A LINKED LIST OF ELEMENTS */
	std::vector < Node > clients;			    // Elements representing clients (clients[0] is a sentinel(�ڱ�) and should not be accessed��ȡ)
	                                                                //���clients�ṹ��clients[i]����ʾ��Ӧ����params->cli[i]����˿ͽڵ�(i>=1),��client IDΪi 
																	// clients[i].whenLastTestedRI��LocalSearch::loadIndividual()�б�ȫ����ʼ��Ϊ-1.

	std::vector < Node > depots;			    // Elements representing depots��ÿ��Ԫ��Ϊ��Ӧ��route��һ���ڱ�(route_iΪ˫����,deports[i]��Ϊָ��route_i��ͷ���header)
	std::vector < Node > depotsEnd;	    // Duplicate of the depots to mark the end of the routes��ÿ��Ԫ��Ϊ��Ӧ��route��һ���ڱ�(depotsEnd[i]Ϊָ��route_i��β���trailer)
	std::vector < Route > routes;				// Elements representing routes

	std::vector < std::vector < ThreeBestInsert > > bestInsertClient;   // (SWAP*) For each route and node, storing the cheapest insertion cost 
	                                                // bestInsertClient�Ĵ�СΪnbVehicles*(nbClients+1), Ԫ��bestInsertClient[Rid][ClientID]��ʾ���ΪClientID�Ĺ˿ͽ��Ҫ���뵽��Rid��routeʱ��ѡ�����3����ѡλ��

	/* TEMPORARY VARIABLES (��ʱ����) USED IN THE LOCAL SEARCH LOOPS */
	// routeU: nodeUPrev -> nodeU -> nodeX -> nodeXNext
	// routeV: nodeVPrev -> nodeV -> nodeY -> nodeYNext
	// routeU:  --------UPrev----------U----------X----------XNext----------
	// routeV:  --------VPrev----------V-----------Y----------YNext----------
	Node * nodeU ;
	Node * nodeX ;
    Node * nodeV ;
	Node * nodeY ;
	Route * routeU ;
	Route * routeV ;
	int nodeUPrevIndex, nodeUIndex, nodeXIndex, nodeXNextIndex ;	
	int nodeVPrevIndex, nodeVIndex, nodeYIndex, nodeYNextIndex ;	
	double loadU, loadX, loadV, loadY;     //ָ������һ���˿ͽڵ�Ļ������������ۻ���
	double serviceU, serviceX, serviceV, serviceY;    //ָ������һ���˿ͽڵ�ķ���ʱ�䣬�����ۻ���
	std::vector<rbp::RectSize> rectsU, rectsX, rectsV, rectsY;
	double penaltyCapacityLS, penaltyDurationLS, penaltyPackLS; //�ͷ�Ȩֵ

	void setLocalVariablesRouteU(); // Initializes some local variables and distances associated to routeU to avoid always querying the same values in the distance matrix
	void setLocalVariablesRouteV(); // Initializes some local variables and distances associated to routeV to avoid always querying the same values in the distance matrix

	/* ----------------------------Functions in charge of excess load and duration penalty calculations----------------------------------------------------------------------------------------*/
	//1. ���� Durantion Լ��
	inline double penaltyExcessDuration(double myDuration) {
		if (!params->durationExist)  return 0.;
		return std::max<double>(0., myDuration - params->durationLimit) * penaltyDurationLS;
	}

	//2. ����װ�� bin Packing Լ��
	inline double penaltyExcessPack(std::vector<rbp::RectSize>& rectList) { return params->bin.binPackingExcess(rectList, params->binPackingExist) * penaltyPackLS;}
	
	//3. �������� Load Լ��
	inline double penaltyExcessLoad(double myLoad) { return std::max<double>(0., myLoad - params->vehicleCapacity)*penaltyCapacityLS;}

	// (Legacy notations: move1...move9 from Prins 2004)
    //  routeU:  --------UPrev----------U----------X----------XNext----------
	//  routeV:  --------VPrev----------V-----------Y----------YNext----------

	/* RELOCATE MOVES */
	bool move1 (); // If U is a client node, remove U and insert it after V
	bool move2 (); // If U and X are client nodes, remove them and insert (U,X) after V
	bool move3 (); // If U and X are client nodes, remove them and insert (X,U) after V

	/* SWAP MOVES */
	bool move4 (); // If U and V are client nodes, swap U and V
	bool move5 (); // If U, X and V are client nodes, swap (U,X) and V
	bool move6 (); // If (U,X) and (V,Y) are client nodes, swap (U,X) and (V,Y) 
	 
	/* 2-OPT and 2-OPT* MOVES */
	bool move7 (); // If route(U) == route(V), replace (U,X) and (V,Y) by (U,V) and (X,Y)�� X��V��һ�η�ת
	bool move8 (); // If route(U) != route(V), replace (U,X) and (V,Y) by (U,V) and (X,Y): route(U)�ĺ��η�ת��Ϊroute(V)��ǰ��Σ�route(V)��ǰ��η�ת��Ϊroute(U)�ĺ���
	bool move9 (); // If route(U) != route(V), replace (U,X) and (V,Y) by (U,Y) and (V,X)

	/* SUB-ROUTINES FOR EFFICIENT SWAP* EVALUATIONS */
	bool swapStar(); // Calculates all SWAP* between routeU and routeV and apply the best improving move
	double getCheapestInsertSimultRemoval(Node * U, Node * V, Node *& bestPosition); // Calculates the insertion cost and position in the route of V, where V is omitted
	void preprocessInsertions(Route * R1, Route * R2); // Preprocess all insertion costs of nodes of route R1 in route R2
	                                                                                           //���R1�е�ÿһ���˿ͽ��U���ҵ�������뵽R2������Ӧ��������Ѵ�����λ��
	/* ROUTINES TO UPDATE THE SOLUTIONS */
	static void insertNode(Node * U, Node * V);		// Solution update: Insert U after V
	static void swapNode(Node * U, Node * V) ;		// Solution update: Swap U and V							   
	void updateRouteData(Route * myRoute);			// Updates the preprocessed data of a route��ÿ��route�Ľṹ�����仯�������ʽ����ˣ����������Ϣ��ͨ�����������ʵ�ָ���
	                                                                                    //��updateRouteData()�����У���ͨ��myRoute��ͷ�ڱ�ȥ�������route�еĽ�㣬ÿ����һ����㣬����¸ý��������Ϣ
	                                                                                    //�ڸ������route�еĸ���������Ϣ֮�����͸��¸�route�������Ϣ
	public:

	// Run the local search with the specified penalty values
	void run(Individual * indiv, double penaltyCapacityLS, double penaltyDurationLS, double penaltyPackLS);

	// Loading an initial solution into the local search
	void loadIndividual(Individual * indiv);

	// Exporting the LS solution into an individual and calculating the penalized cost according to the original penalty weights from Params
	//����������ڽ�Local Search�ڲ�����vector���͵�clients, depots,depotsEnd,routes�ṹ���洢�Ľ��ȡ����������ÿ��route������route��ƽ�����ĵĻ��ȴ�С��������,
	//Ȼ�����θ����ⲿ��indiv���壬������chromR��chromeT�ṹ��Ȼ��ø���ͨ������������ĳ�Ա����indiv->evaluateCompleteCost()��ʵ����Ӧ����ֵ��ƥ�����.
	void exportIndividual(Individual * indiv);

	// Constructor
	LocalSearch(Params * params);
};

#endif
