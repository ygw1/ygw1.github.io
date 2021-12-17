#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Individual.h"
#include "MaxRectsBinPack.h"

struct Node ;

// Structure containing a route
struct Route
{
	int cour;							                // Route index, 从0开始算, 到 params->nbVehicles-1
	int nbCustomers;					        // Number of customers visited in the route
	int whenLastModified;				    // "When" this route has been last modified
	int whenLastTestedSWAPStar;   // "When" the SWAP* moves for this route have been last tested
	Node * depot;						        // Pointer to the associated depot, 该route的头哨兵结点(哨兵结点不存内容，只起引导作用)

	double duration;					       // Total time spent on the route
	double load;						           // Total load on the route
	std::vector<rbp::RectSize> rectsList;  // Total rectangles on the route

	double reversalDistance;		     // Difference of cost if the route is reversed.
														 // 这个参数表示，如果将这条路逆着走，那么同顺着走相比，产生的差异：distance(逆着走)-distance(顺着走)
	double penalty;						     // Current sum of load, duration, and bin packing penalties，这个只是超过的部分的惩罚值，不包含行驶距离.
														 // myRoute->penalty = penaltyExcessDuration(mytime) + penaltyExcessLoad(myload) + penaltyExcessPack(rectList)
	double polarAngleBarycenter;// Polar angle of the barycenter(重心) of the route，用的是弧度表示的.
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
	double cumulatedReversalDistance;	// Difference of cost if the segment of route [0,1,...,cour] is reversed (useful for 2-opt moves with asymmetric problems非对称问题)
																	// 这个参数表示，如果从当前结点逆着走回出发点，那么同顺着从出发点走到此，产生的差异: distance(cour,...,1,0) - distance(0,1,...,cour)
	std::vector<rbp::RectSize> cumulatedRectsList;  // Cumulated rectangles on this route from the first customer to this customer (including itself)

	std::vector<rbp::RectSize> remainderRectsList;  // Cumulated rectangles on this route from this customer to the final customer (including itself)

	double deltaRemoval;		// Difference of distance in the current route if the node is removed (used in SWAP*)：deltaRemoval = distance_new - distance_old
												// 注意：这个仅仅是计算将当前node从所在route中移走这个结点所带来的距离的变化值（注意：只是距离值的变化）
	                                            // 每个结点的deltaRemoval是在LocalSearch::preprocessInsertions() 函数得以计算的，which is useful for swap*.
};

// Structure used in SWAP* to remember the three best insertion positions of a customer in a given route
struct ThreeBestInsert
{
	int whenLastCalculated;
	double bestCost[3];           //bestCost[i]指的是将结点插入到bestLocation[i]所指的结点的身后，被插入的route在distance上的的变化
	Node * bestLocation[3];   //bestLocation[i]指的是候选的插入位置为bestLocation[i]所指的结点的身后

	//将当前Node插入到placeInsert这个结点之后，对应的distance变化为costInsert，然后，
	//将这种变化同该Node此前的三个最佳候选位置进行比, 更新最佳候选插入位置
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
	//moveCost表示将U和V分别插入到bestPositionU和bestPositionV身后，带来的两条路线各自在(totalDistance + 各个属性的惩罚值 )的前后变化值
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
												// 每当localSearch对象使用loadIndividual()加载一个新的indiv进行处理的时候, loadIndividual()函数会将nbMoves设为0.
												// Attention: this is not only a simple counter, it is also used to avoid repeating move evaluations.
	std::vector < int > orderNodes;			// Randomized order for checking the nodes in the RI local search，即对1,2,3,...,nbClients 的一个乱序.
	std::vector < int > orderRoutes;			// Randomized order for checking the routes in the SWAP* local search，即对0,1,2,3,...,(nbVehicles-1) 的一个乱序.
	std::set < int > emptyRoutes;				// indices of all empty routes
	int loopID;									// Current loop index

	/* THE SOLUTION IS REPRESENTED AS A LINKED LIST OF ELEMENTS */
	std::vector < Node > clients;			    // Elements representing clients (clients[0] is a sentinel(哨兵) and should not be accessed存取)
	                                                                //这个clients结构，clients[i]即表示对应的是params->cli[i]这个顾客节点(i>=1),即client ID为i 
																	// clients[i].whenLastTestedRI在LocalSearch::loadIndividual()中被全部初始化为-1.

	std::vector < Node > depots;			    // Elements representing depots，每个元素为对应的route的一个哨兵(route_i为双链表,deports[i]即为指向route_i的头结点header)
	std::vector < Node > depotsEnd;	    // Duplicate of the depots to mark the end of the routes，每个元素为对应的route的一个哨兵(depotsEnd[i]为指向route_i的尾结点trailer)
	std::vector < Route > routes;				// Elements representing routes

	std::vector < std::vector < ThreeBestInsert > > bestInsertClient;   // (SWAP*) For each route and node, storing the cheapest insertion cost 
	                                                // bestInsertClient的大小为nbVehicles*(nbClients+1), 元素bestInsertClient[Rid][ClientID]表示序号为ClientID的顾客结点要插入到第Rid条route时可选的最佳3个候选位置

	/* TEMPORARY VARIABLES (临时变量) USED IN THE LOCAL SEARCH LOOPS */
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
	double loadU, loadX, loadV, loadY;     //指的是这一个顾客节点的货物量，而非累积量
	double serviceU, serviceX, serviceV, serviceY;    //指的是这一个顾客节点的服务时间，而非累积量
	std::vector<rbp::RectSize> rectsU, rectsX, rectsV, rectsY;
	double penaltyCapacityLS, penaltyDurationLS, penaltyPackLS; //惩罚权值

	void setLocalVariablesRouteU(); // Initializes some local variables and distances associated to routeU to avoid always querying the same values in the distance matrix
	void setLocalVariablesRouteV(); // Initializes some local variables and distances associated to routeV to avoid always querying the same values in the distance matrix

	/* ----------------------------Functions in charge of excess load and duration penalty calculations----------------------------------------------------------------------------------------*/
	//1. 关于 Durantion 约束
	inline double penaltyExcessDuration(double myDuration) {
		if (!params->durationExist)  return 0.;
		return std::max<double>(0., myDuration - params->durationLimit) * penaltyDurationLS;
	}

	//2. 关于装箱 bin Packing 约束
	inline double penaltyExcessPack(std::vector<rbp::RectSize>& rectList) { return params->bin.binPackingExcess(rectList, params->binPackingExist) * penaltyPackLS;}
	
	//3. 关于载重 Load 约束
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
	bool move7 (); // If route(U) == route(V), replace (U,X) and (V,Y) by (U,V) and (X,Y)： X到V这一段反转
	bool move8 (); // If route(U) != route(V), replace (U,X) and (V,Y) by (U,V) and (X,Y): route(U)的后半段反转作为route(V)的前半段，route(V)的前半段反转作为route(U)的后半段
	bool move9 (); // If route(U) != route(V), replace (U,X) and (V,Y) by (U,Y) and (V,X)

	/* SUB-ROUTINES FOR EFFICIENT SWAP* EVALUATIONS */
	bool swapStar(); // Calculates all SWAP* between routeU and routeV and apply the best improving move
	double getCheapestInsertSimultRemoval(Node * U, Node * V, Node *& bestPosition); // Calculates the insertion cost and position in the route of V, where V is omitted
	void preprocessInsertions(Route * R1, Route * R2); // Preprocess all insertion costs of nodes of route R1 in route R2
	                                                                                           //针对R1中的每一个顾客结点U，找到将其插入到R2中所对应的三个最佳待插入位置
	/* ROUTINES TO UPDATE THE SOLUTIONS */
	static void insertNode(Node * U, Node * V);		// Solution update: Insert U after V
	static void swapNode(Node * U, Node * V) ;		// Solution update: Swap U and V							   
	void updateRouteData(Route * myRoute);			// Updates the preprocessed data of a route，每次route的结构发生变化，即访问结点变了，它的相关信息将通过这个函数来实现更新
	                                                                                    //在updateRouteData()函数中，会通过myRoute的头哨兵去访问这个route中的结点，每访问一个结点，会更新该结点的相关信息
	                                                                                    //在更新完该route中的各个结点的信息之后，最后就更新该route的相关信息
	public:

	// Run the local search with the specified penalty values
	void run(Individual * indiv, double penaltyCapacityLS, double penaltyDurationLS, double penaltyPackLS);

	// Loading an initial solution into the local search
	void loadIndividual(Individual * indiv);

	// Exporting the LS solution into an individual and calculating the penalized cost according to the original penalty weights from Params
	//这个函数用于将Local Search内部基于vector类型的clients, depots,depotsEnd,routes结构所存储的解读取出来，并将每个route依照其route的平均质心的弧度从小到大排序,
	//然后依次赋给外部的indiv个体，更新其chromR和chromeT结构，然后该个体通过调用其自身的成员函数indiv->evaluateCompleteCost()来实现相应属性值的匹配更新.
	void exportIndividual(Individual * indiv);

	// Constructor
	LocalSearch(Params * params);
};

#endif
