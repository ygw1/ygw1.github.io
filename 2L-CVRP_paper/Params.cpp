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
	isRoundingInteger = false; //四舍五入整数，就是在开更号计算两个顾客的距离的时候，是否要把距离转为整数

	// Initialize RNG
	srand(commandline->seed);//在这里设置随机数种子，后面用到的，比如std::random_shuffle(chromT.begin(), chromT.end())，就是默认使用srand()所设置的种子			

	// Read INPUT dataset
	std::ifstream inputFile(commandline->pathInstance);
	if (inputFile.is_open())
	{
		// Reading nbClients, nbVehicles, vehicleCapacity, vehicleHeight, vehicleWidth:
		getline(inputFile, content);//利用getline()函数，逐行读取，行与行之间用回车符区分
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
			inputFile >> cli[i].custNum >> cli[i].coordX >> cli[i].coordY>>cli[i].demand; //利用">>"实现逐次赋值每调用一次>>运算符函数，inputFile对象的文件流指针往后移动
			if (cli[i].demand > maxDemand) maxDemand = cli[i].demand;
			totalDemand += cli[i].demand;
			cli[i].polarAngle = CircleSector::positive_mod(32768. * atan2(cli[i].coordY - cli[0].coordY, cli[i].coordX - cli[0].coordX) / PI);
			// 注意，std::getline()的文件流指针和std::ifstream流对象的指针不同，让std::getline()的文件流指针也跟随ifstream流对象的文件流指针同步移动
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
	timeCost = std::vector < std::vector< double > >(nbClients + 1, std::vector <double>(nbClients + 1));//初始化
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
	// 对上面的距离矩阵做处理: backhaul->linehaul,设为INFINITE_DISTANCE，0->Backhaul也设为INFINITE_DISTANCE

	// Calculation of the correlated vertices for each customer (for the granular restriction)
	correlatedVertices = std::vector < std::vector < int > >(nbClients + 1);
	std::vector < std::set < int > > setCorrelatedVertices = std::vector < std::set <int> >(nbClients + 1); //setCorrelatedVertices[i]表示ID为i的顾客的关联邻居的ID集合
	std::vector < std::pair <double, int> > orderProximity;
	for (int i = 1; i <= nbClients; i++)//只针对client客户才会有邻居的概念，deport是没有邻居的
	{
		orderProximity.clear();
		for (int j = 1; j <= nbClients; j++)
			if (i != j) orderProximity.push_back(std::pair <double, int>(timeCost[i][j], j));
		std::sort(orderProximity.begin(), orderProximity.end()); //升序排列

		for (int j = 0; j < std::min<int>(nbGranular, nbClients - 1); j++)
		{
			// If i is correlated with j, then j should be correlated with i.作者加了前面这个规则，使得每个顾客的邻居的数目变大了
			setCorrelatedVertices[i].insert(orderProximity[j].second);
			setCorrelatedVertices[orderProximity[j].second].insert(i); //如果不考虑“If i is correlated with j, then j should be correlated with i”
		}															                                                                               //这条语句就需要注释掉.这条规则适用于解决对称问题(即，C_i_j=C_j_i )
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