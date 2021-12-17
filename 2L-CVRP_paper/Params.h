
#ifndef PARAMS_H
#define PARAMS_H

#include "CircleSector.h"
#include <string>
#include <vector>
#include <list>
#include <set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <time.h>
#include <climits>
#include <algorithm>
#include <unordered_set>
#include "MaxRectsBinPack.h"
#include "commandline.h"
#define MY_EPSILON 0.00001 // Precision parameter, used to avoid numerical instabilities
#define PI 3.14159265359
#define INFINITE_DISTANCE 1000000  //�����趨backhaul��linehaul�ľ���

struct Client
{
	int custNum;			    // Index of the customer(customer ID)
	double coordX;		   	// Coordinate X
	double coordY;			// Coordinate Y
	double serviceDuration; // Service duration
	double demand;			   // Demand
	bool isLinehaul = true;    //indicates the customer type: linehaul or backhaul.
	std::vector<rbp::RectSize> rects;
	int polarAngle;			// Polar angle of the client around the depot, measured in degrees and truncated for convenience
	                                    //����һ����(x,y)�Ļ���Ϊatan2(y, x) �����Ӧ�ĽǶȣ���360���׼�£�Ϊ180*atan2(y, x) / PI * 180��
									    //�ڱ������ж�Ӧ��ת����ļ��ǣ�65536���׼�£�Ϊpositive_mod(32768.*atan2(y, x) / PI),���У�positive_mod()������Ϊ�˽��Ƕ�ͳһת��Ϊ��ֵ.
};

class Params
{
public:
	CommandLine* commandline;

	/* PARAMETERS OF THE GENETIC ALGORITHM */
	int nbGranular	= 20;	// Granular search parameter, limits the number of moves in the RI local search
	int mu					= 25;	// Minimum population size
	int lambda			= 40;	// Number of solutions created before reaching the maximum population size (i.e., generation size)
	int nbElite			= 4;		// Number of elite individuals (reduced in HGS-2020)
	int nbClose			= 5;		// Number of closest solutions/individuals considered when calculating diversity contribution
											// ���������Individual::averageBrokenPairsDistanceClosest(nbClose)��ʹ��
	double targetFeasible   = 0.2;		// Reference proportion for the number of feasible individuals, used for the adaptation of the penalty parameters

	/* parameters of bin packing*/
	double vehicleWidth=0;
	double vehicleHeight=0;
	bool     allowFlip;    //initialized in the initialization list.
	rbp::MaxRectsBinPack bin;

	/* ADAPTIVE PENALTY COEFFICIENTS */
	double penaltyCapacity;				// Penalty for one unit of capacity excess (adapted through the search)
	double penaltyDuration;				// Penalty for one unit of duration excess (adapted through the search)
	double penaltyPack;                    // penalty for bin packing feasibility

	/* DATA OF THE PROBLEM INSTANCE */			
	bool isRoundingInteger ;							// Distance calculation convention ���������������������������˿͵ľ����ʱ���Ƿ�Ҫ�Ѿ���תΪ����
	bool isDurationConstraint=false ;			// Indicates if the problem includes duration constraints
	int nbClients ;											// Number of clients (excluding the depot)
	int nbVehicles= INT_MAX;						// Number of vehicles����ʼ��Ϊһ���ܴ����
	double durationLimit;								// Route duration limit
	double vehicleCapacity;							// Capacity limit
	double totalDemand ;								// Total demand required by all the clients
	double maxDemand;								// Maximum demand of a client
	double maxDist;										// Maximum distance between two clients
	std::vector < Client > cli ;							// Vector containing information on each client��������deport���
	std::vector < std::vector < double > > timeCost ;		    // Distance matrix:(nbClient+1)^2
	std::vector < std::vector < int > > correlatedVertices;	// Neighborhood restrictions: For each client, list of nearby customers

	/*�����Լ������*/
	bool durationExist=false;
	bool binPackingExist = false;
	// Initialization from a given data set
	Params(CommandLine* commandline);

	static unsigned long long testBug1;
	static unsigned long long testBug2;
	static unsigned long long testBug3;
};
#endif

