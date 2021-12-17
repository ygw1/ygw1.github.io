#include "Params.h"

Params::Params(CommandLine* commandline):commandline(commandline), allowFlip(commandline->allowFlip)
{
	std::string content, content2, content3;
	double serviceTimeData = 0.;
	nbClients = 0;
	totalDemand = 0.;
	maxDemand = 0.;
	maxDist = 0.;
	durationLimit = 1.e30;
	vehicleCapacity = 1.e30;
	isRoundingInteger = false; //�������������������ڿ����ż��������˿͵ľ����ʱ���Ƿ�Ҫ�Ѿ���תΪ����

	// Initialize RNG
	srand(commandline->seed);//������������������ӣ������õ��ģ�����std::random_shuffle(chromT.begin(), chromT.end())������Ĭ��ʹ��srand()�����õ�����			

	// Read INPUT dataset
	std::ifstream inputFile(commandline->pathInstance);
	if (inputFile.is_open())
	{
		// Reading nbClients, nbVehicles, vehicleCapacity, vehicleHeight, vehicleWidth:
		getline(inputFile, content);//����getline()���������ж�ȡ��������֮���ûس�������
		getline(inputFile, content);
		getline(inputFile, content);
		std::stringstream strObj(content);
		strObj >> nbClients;
		getline(inputFile, content);
		std::stringstream strObj1(content);
		strObj1 >> nbVehicles;
		getline(inputFile, content);
		getline(inputFile, content);
		getline(inputFile, content);
		std::stringstream strObj2(content);
		strObj2 >> vehicleCapacity >> vehicleHeight >> vehicleWidth;
		getline(inputFile, content);

		// Reading coordinates and demands information of every single client:
		cli = std::vector<Client>(nbClients + 1);
		for (int i = 0; i <= nbClients; i++)
		{
			inputFile >> cli[i].custNum >> cli[i].coordX >> cli[i].coordY>>cli[i].demand; //����">>"ʵ����θ�ֵÿ����һ��>>�����������inputFile������ļ���ָ�������ƶ�
			if (cli[i].demand > maxDemand) maxDemand = cli[i].demand;
			totalDemand += cli[i].demand;
			cli[i].polarAngle = CircleSector::positive_mod(32768. * atan2(cli[i].coordY - cli[0].coordY, cli[i].coordX - cli[0].coordX) / PI);
			// ע�⣬std::getline()���ļ���ָ���std::ifstream�������ָ�벻ͬ����std::getline()���ļ���ָ��Ҳ����ifstream��������ļ���ָ��ͬ���ƶ�
			getline(inputFile, content);
		}

		// Reading rectangles information of every single client.
		getline(inputFile, content);// skip a line
		for (int i = 0; i <= nbClients; i++) {
			getline(inputFile, content);
			std::stringstream strObj(content);
			int id, nbRects;
			strObj >> id >> nbRects;
			cli[id].rects = std::vector<rbp::RectSize>(nbRects);
			for (int i = 0; i <nbRects; i++) {
				strObj >> cli[id].rects[i].height >> cli[id].rects[i].width;
				cli[id].rects[i].clientID = id;
			}
		}

	// Default initialization if the number of vehicles has not been provided by the user
	if (nbVehicles == INT_MAX)
	{
		nbVehicles = std::ceil(1.2 * totalDemand / vehicleCapacity) + 2;  // Safety margin: 20% + 2 more vehicles than the trivial bin packing LB
		std::cout << "----- FLEET SIZE WAS NOT SPECIFIED. DEFAULT INITIALIZATION TO: " << nbVehicles << std::endl;
	}

	// Calculation of the distance matrix
	timeCost = std::vector < std::vector< double > >(nbClients + 1, std::vector <double>(nbClients + 1));//��ʼ��
	for (int i = 0; i <= nbClients; i++)
	{
		for (int j = 0; j <= nbClients; j++)
		{
			double d = std::sqrt((cli[i].coordX - cli[j].coordX) * (cli[i].coordX - cli[j].coordX) + (cli[i].coordY - cli[j].coordY) * (cli[i].coordY - cli[j].coordY));
			if (isRoundingInteger) { d += 0.5; d = (double)(int)d; } // integer rounding
			if (d > maxDist) maxDist = d;
			timeCost[i][j] = d;
		}
	}
	// consider the circumstance: backhauls
	// ������ľ������������: backhaul->linehaul,��ΪINFINITE_DISTANCE��0->BackhaulҲ��ΪINFINITE_DISTANCE

	// Calculation of the correlated vertices for each customer (for the granular restriction)
	correlatedVertices = std::vector < std::vector < int > >(nbClients + 1);
	std::vector < std::set < int > > setCorrelatedVertices = std::vector < std::set <int> >(nbClients + 1); //setCorrelatedVertices[i]��ʾIDΪi�Ĺ˿͵Ĺ����ھӵ�ID����
	std::vector < std::pair <double, int> > orderProximity;
	for (int i = 1; i <= nbClients; i++)//ֻ���client�ͻ��Ż����ھӵĸ��deport��û���ھӵ�
	{
		orderProximity.clear();
		for (int j = 1; j <= nbClients; j++)
			if (i != j) orderProximity.push_back(std::pair <double, int>(timeCost[i][j], j));
		std::sort(orderProximity.begin(), orderProximity.end()); //��������

		for (int j = 0; j < std::min<int>(nbGranular, nbClients - 1); j++)
		{
			// If i is correlated with j, then j should be correlated with i.���߼���ǰ���������ʹ��ÿ���˿͵��ھӵ���Ŀ�����
			setCorrelatedVertices[i].insert(orderProximity[j].second);
			setCorrelatedVertices[orderProximity[j].second].insert(i); //��������ǡ�If i is correlated with j, then j should be correlated with i��
		}															                                                                               //����������Ҫע�͵�.�������������ڽ���Գ�����(����C_i_j=C_j_i )
	}

	// Filling the vector of correlated vertices
	for (int i = 1; i <= nbClients; i++)
		for (int x : setCorrelatedVertices[i])
			correlatedVertices[i].push_back(x);


	/* parameters setting for the initial values of the penalties */
	penaltyDuration = 1;
	penaltyPack = 100;
	penaltyCapacity = 1;

	/* parameters setting for the two-dimensional bin packing */
	bin.Init(this->vehicleWidth, this->vehicleHeight, this->allowFlip);

	}

}

unsigned long long Params::testBug1 = 0;
unsigned long long Params::testBug2 = 0;
unsigned long long Params::testBug3 = 0;