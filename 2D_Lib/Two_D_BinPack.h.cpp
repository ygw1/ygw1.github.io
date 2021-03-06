//Author:Yang Guangwang <ygw@mail.ustc.edu.cn>
//University of Science and Technology of China
//Copyright (c) Yangguang Wang

#include <algorithm>
#include <utility>
#include <iostream>
#include <limits>
#include <cassert>
#include <cstring>
#include "Two_D_BinPack.h"
namespace rbp {

	using namespace std;

	MaxRectsBinPack::MaxRectsBinPack()
		:binWidth(0),
		binHeight(0)
	{
	}

	MaxRectsBinPack::MaxRectsBinPack(int width, int height, bool allowFlip)
	{
		Init(width, height, allowFlip);
	}

	//默认是允许旋转的
	void MaxRectsBinPack::Init(int width, int height, bool allowFlip)
	{
		binAllowFlip = allowFlip;
		binWidth = width;
		binHeight = height;

		Rect n;// 代表箱子
		n.x = 0;
		n.y = 0;
		n.width = width;
		n.height = height;

		usedRectangles.clear();

		freeRectangles.clear();
		freeRectangles.push_back(n);
	}

	Rect MaxRectsBinPack::Insert(int width, int height, FreeRectChoiceHeuristic method)
	{
		Rect newNode;
		// Unused in this function. We don't need to know the score after finding the position.
		int score1 = std::numeric_limits<int>::max();
		int score2 = std::numeric_limits<int>::max();
		switch (method)
		{
		case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
		case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
		case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint(width, height, score1); break;
		case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
		case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
		}
		//如果无法放置该Rectangle,则直接返回即可
		if (newNode.height == 0)
			return newNode;
		//如果该item能够被放到某个freeRect里面，则更新整个free Rect List
		size_t numRectanglesToProcess = freeRectangles.size();
		for (size_t i = 0; i < numRectanglesToProcess; ++i)
		{
			if (SplitFreeNode(freeRectangles[i], newNode))
			{
				freeRectangles.erase(freeRectangles.begin() + i);
				--i;
				--numRectanglesToProcess;
			}
		}

		PruneFreeList();

		usedRectangles.push_back(newNode);
		return newNode;
	}

	void MaxRectsBinPack::Insert(std::vector<RectSize>& rects, std::vector<Rect>& dst, FreeRectChoiceHeuristic method)
	{
		dst.clear();

		while (rects.size() > 0)
		{
			int bestScore1 = std::numeric_limits<int>::max();
			int bestScore2 = std::numeric_limits<int>::max();
			int bestRectIndex = -1;
			Rect bestNode;

			for (size_t i = 0; i < rects.size(); ++i)
			{
				int score1;
				int score2;
				Rect newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);

				if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
				{
					bestScore1 = score1;
					bestScore2 = score2;
					bestNode = newNode;
					bestRectIndex = i;
				}
			}

			if (bestRectIndex == -1)
				return;

			PlaceRect(bestNode);
			dst.push_back(bestNode);
			rects.erase(rects.begin() + bestRectIndex);
		}
	}

	//总调用函数:
	/*调用方式一：每次要检查装箱是否可行时，生成一个bin对象，再来检验
	rbp::MaxRectsBinPack bin(width, height, allowFlip);
	bool result = bin.binPackingCheck(rectSeq); */
	/*调用方式二：
	1. 当问题为同构车队时，为了减少不必要的时间消耗，可以将一个初始化后的bin对象作为成员变量放在Local Search类中，rbp::MaxRectsBinPack bin(width, height, allowFlip);
	当需要使用时，再调用成员函数即可，譬如：bool result = bin.binPackingCheck(rectSeq);
	2. 当问题为异构车队时，同样可以将一个初始化后的bin对象作为成员变量放在Local Search类中，rbp::MaxRectsBinPack bin;
	当需要使用时，先初始化箱子的规模，再调用其成员函数即可：bin.Init(width, height, allowFlip); bool result = bin.binPackingCheck(rectSeq);*/

	bool MaxRectsBinPack::binPackingCheck(vector<RectSize>& rectSeq) {
		std::set<int> routeClientSet;
		for (auto iter : rectSeq) { routeClientSet.insert(iter.clientID); } //note that the elements in the set container has already been sorted

		std::string routeIDStr;
		for (auto ID : routeClientSet)
		{
			std::string tempStr = std::to_string(ID);
			std::string strID = tempStr.size() == 3 ? tempStr : (tempStr.size() == 2 ? "0" + tempStr : "00" + tempStr);
			routeIDStr += strID;
		}
		auto iter = this->routeCheckMap.find(routeIDStr);
		if (iter != this->routeCheckMap.end())
			return iter->second;
		else {
			/*这是单一方法*/
			//bool packResult = multiPack(rectSeq); //这里修改所要调用的装箱函数
			//this->routeCheckMap.insert({ routeIDStr,packResult });
			//return packResult;

			/*这是组合方法*/
			bool packResult = multiPack(rectSeq);
			if (packResult) {
				this->routeCheckMap.insert({ routeIDStr,packResult });
				return packResult;
			}
			else {
				packResult = myHeuristicPack(rectSeq);
				this->routeCheckMap.insert({ routeIDStr,packResult });
				return packResult;
			}
		}




		//在Local search环境中定义一个MaxRectsBinPack类型的对象：rbp::MaxRectsBinPack bin;
	   //后面每次使用时先initiate一下，然后再调用: 
	   // @step 1: bin.Init(Params::vehicle_width, Params::vehicle_height, Params::allowRotation); 
		//@step 2: bool heuristicResult = bin.myHeuristicPack(bin, rectSequence);

		//bool basicPackResult = basicPack(rectSeq);
		//if (basicPackResult)
		//	return true;

//	    bool basicRandomisedResult = basicRandomisedPack(rectSeq);
//		if (basicRandomisedResult)
//			return true;

		//bool multiPackResult = multiPack(rectSeq); //这个参数是值传递
		//if (multiPackResult)
		//	return true;

		//bool randomResult= randomLSPack(rectSeq);
		//if (randomResult)
		//	return true;

		//bool heuristicResult = myHeuristicPack(rectSeq);
		//if (heuristicResult)
		//	return true;

		//if all the methods fail to pack all the rectangles into the bin, the function will return false.
		//return false;
	}

	double MaxRectsBinPack::binPackingExcess(vector<rbp::RectSize>& rectList, bool binPackingExist) {
		if (rectList.size() == 0) return 0;
		//Case 1: do not consider the bin packing constraints
		if (!binPackingExist) return 0;
		//Case 2: consider the bin packing circumstance,but not includes the backhauls
		double totalArea = 0;
		for (const auto iter : rectList) { totalArea += iter.height * iter.width; }
		double nbBin = totalArea / ((double)this->binHeight * (double)this->binWidth);
		int lowerBound = ceil(nbBin);
		if (lowerBound > 1) return lowerBound;
		else return this->binPackingCheck(rectList) ? 0 : 1;

	}



	bool MaxRectsBinPack::basicPack(vector<rbp::RectSize> rectSequence) {
		vector<RectSize> inputSeq;
		vector<Rect> outputSeq;

		//the  sequence biased MaxRectsBinPack METHODS
		for (int sortRule = 1; sortRule <= 3; ++sortRule)
		{
			//first, set a sequence followed by sortRule
			if(sortRule==1)  sort(rectSequence.begin(), rectSequence.end(), ComparatorArea());
			else if(sortRule==2) sort(rectSequence.begin(), rectSequence.end(), ComparatorWidth());
			else if(sortRule==3) sort(rectSequence.begin(), rectSequence.end(), ComparatorHeight());

			//second, pack the rectangles with different placeRule.
			switch (1)
			{//从case1依次执行到最后一个case
			case 1: {
				inputSeq = rectSequence;
				this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
				bool feasibility = true;
				for (int i = 0; i < inputSeq.size(); ++i) 
				{
					// Perform the packing.
					MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBottomLeftRule;
					Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
					// Test success or failure.
					if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
						feasibility = false;
						break;
					}
				}
				if (feasibility)
					return true;
			}

			case 5: {
				inputSeq = rectSequence;
				this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
				bool feasibility = true;
				for (int i = 0; i < inputSeq.size(); ++i) {
					// Perform the packing.
					MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectContactPointRule;
					Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
					// Test success or failure.
					if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
						feasibility = false;
						break;
					}
				}
				if (feasibility)
					return true;
			}

			case 2: {
				inputSeq = rectSequence;
				this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
				bool feasibility = true;
				for (int i = 0; i < inputSeq.size(); ++i) {
					// Perform the packing.
					MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestAreaFit;
					Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
					// Test success or failure.
					if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
						feasibility = false;
						break;
					}
				}
				if (feasibility)
					return true;
			}

			case 3: {
				inputSeq = rectSequence;
				this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
				bool feasibility = true;
				for (int i = 0; i < inputSeq.size(); ++i) {
					// Perform the packing.
					MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestShortSideFit;
					Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
					// Test success or failure.
					if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
						feasibility = false;
						break;
					}
				}
				if (feasibility)
					return true;
			}

			case 4: {
				inputSeq = rectSequence;
				this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
				bool feasibility = true;
				for (int i = 0; i < inputSeq.size(); ++i) {
					// Perform the packing.
					MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestLongSideFit;
					Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
					// Test success or failure.
					if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
						feasibility = false;
						break;
					}
				}
				if (feasibility)
					return true;
			}

			}
		}
		return false;
	}

	// basic packing methods + randomised-MTP
	bool MaxRectsBinPack::basicRandomisedPack(vector<rbp::RectSize> rectSequence) {
		vector<RectSize> inputSeq;
		vector<Rect> outputSeq;

		//the  sequence biased MaxRectsBinPack METHODS
		for (int sortRule = 1; sortRule <= 3; ++sortRule) {
			//first, set a sequence followed by sortRule
			switch (sortRule) {
			case 1:sort(rectSequence.begin(), rectSequence.end(), ComparatorArea()); break;
			case 2:sort(rectSequence.begin(), rectSequence.end(), ComparatorWidth()); break;
			case 3:sort(rectSequence.begin(), rectSequence.end(), ComparatorHeight()); break;
			}

			//second, pack the rectangles with different placeRule.
			for (int placeRule = 1; placeRule <= 5; ++placeRule) {
				switch (placeRule) {
				case 1: {
					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBottomLeftRule;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 5: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectContactPointRule;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 2: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestAreaFit;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 3: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestShortSideFit;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 4: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestLongSideFit;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}

				}
			}

		}

		//baised-randomised MTP method.
		int maxPackIter = rectSequence.size();
		for (int i = 0; i < maxPackIter; ++i) {
			inputSeq = rectSequence;
			outputSeq.clear();
			this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
			this->biasedRandomisedInsert(inputSeq, outputSeq);
			if (inputSeq.size() == 0)
				return true;
		}

		return false;
	}



	bool MaxRectsBinPack::multiPack(vector<rbp::RectSize> rectSequence) {
		vector<RectSize> inputSeq;
		vector<Rect> outputSeq;

		//the  sequence biased MaxRectsBinPack METHODS
		for (int sortRule = 1; sortRule <= 3; ++sortRule) {
			//first, set a sequence followed by sortRule
			switch (sortRule) {
			case 1:sort(rectSequence.begin(), rectSequence.end(), ComparatorArea()); break;
			case 2:sort(rectSequence.begin(), rectSequence.end(), ComparatorWidth()); break;
			case 3:sort(rectSequence.begin(), rectSequence.end(), ComparatorHeight()); break;
			}

			//second, pack the rectangles with different placeRule.
			for (int placeRule = 1; placeRule <= 5; ++placeRule) {
				switch (placeRule) {
				case 1: {
					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBottomLeftRule;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 5: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectContactPointRule;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 2: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestAreaFit;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 3: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestShortSideFit;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}
				case 4: {

					inputSeq = rectSequence;
					this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
					bool feasibility = true;
					for (int i = 0; i < inputSeq.size(); ++i) {
						// Perform the packing.
						MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestLongSideFit;
						Rect packedRect = this->Insert(inputSeq[i].width, inputSeq[i].height, heuristic);
						// Test success or failure.
						if (packedRect.height == 0) {//the current rectangle cannot be packed, the rest rectangles will no need to be examined
							feasibility = false;
							break;
						}
					}

					if (feasibility) {
						return true;
					}
					break;
				}

				}
			}

		}

		//the GLOBAL MaxRectsBinPack METHODS
		//GLOBAL-MTP
		inputSeq = rectSequence;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		this->Insert(inputSeq, outputSeq, MaxRectsBinPack::RectContactPointRule);
		if (inputSeq.size() == 0)
			return true;

		//GLOBAL-MAXRECTS-BL
		inputSeq = rectSequence;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		this->Insert(inputSeq, outputSeq, MaxRectsBinPack::RectBottomLeftRule);
		if (inputSeq.size() == 0)
			return true;

		//GLOBAL-MAXRECTS-BAF
		inputSeq = rectSequence;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		this->Insert(inputSeq, outputSeq, MaxRectsBinPack::RectBestAreaFit);
		if (inputSeq.size() == 0)
			return true;

		//GLOBAL-MAXRECTS-BSSF
		inputSeq = rectSequence;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		this->Insert(inputSeq, outputSeq, MaxRectsBinPack::RectBestShortSideFit);
		if (inputSeq.size() == 0)
			return true;

		//GLOBAL-MAXRECTS-BLSF
		inputSeq = rectSequence;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		this->Insert(inputSeq, outputSeq, MaxRectsBinPack::RectBestLongSideFit);
		if (inputSeq.size() == 0)
			return true;

		//baised-randomised MTP method.
		int maxPackIter = rectSequence.size();
		for (int i = 0; i < maxPackIter; ++i) {
			inputSeq = rectSequence;
			outputSeq.clear();
			this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
			this->biasedRandomisedInsert(inputSeq, outputSeq);
			if (inputSeq.size() == 0)
				return true;
		}

		return false;

	}

	bool MaxRectsBinPack::randomLSPack(vector<rbp::RectSize> rectSequence) {
		//RandomLS procedure:
		vector<RectSize> itemSeq = rectSequence;
		vector<RectSize> inputSeq;
		vector<Rect> outputSeq;
		int usedSurfaceArea = 0;
		int maxOccupyArea = 0;
		int iterNum = rectSequence.size();

		for (int sortRule = 1; sortRule <= 3; ++sortRule) {
			switch (sortRule) {
			case 1:sort(itemSeq.begin(), itemSeq.end(), ComparatorArea()); break;
			case 2:sort(itemSeq.begin(), itemSeq.end(), ComparatorWidth()); break;
			case 3:sort(itemSeq.begin(), itemSeq.end(), ComparatorHeight()); break;
			}
			//初次调用装箱算法.
			inputSeq = itemSeq;
			outputSeq.clear();
			this->heuristicInsert(inputSeq, outputSeq);
			if (inputSeq.size() == 0)
				return true;
			else {
				usedSurfaceArea = 0;
				for (int i = 0; i < outputSeq.size(); ++i) {
					usedSurfaceArea += outputSeq[i].width * outputSeq[i].height;
				}
				maxOccupyArea = usedSurfaceArea;
			}

			for (int nonImproved = 1; nonImproved <= iterNum; ++nonImproved) {
				inputSeq = itemSeq;
				//在itemSeq中随机更换两个item的位置
				static default_random_engine generator;
				std::uniform_int_distribution<int> packDist(0, itemSeq.size() - 1);
				int pos1 = packDist(generator);
				int pos2 = packDist(generator);
				swap(inputSeq[pos1], inputSeq[pos2]);
				vector<RectSize> tempSeq = inputSeq; //保存这个input sequence
				outputSeq.clear();
				this->heuristicInsert(inputSeq, outputSeq);
				if (inputSeq.size() == 0)
					return true;
				else {
					usedSurfaceArea = 0;//必须先清零，否则出错
					for (int i = 0; i < outputSeq.size(); ++i) {
						usedSurfaceArea += outputSeq[i].width * outputSeq[i].height;
					}
					if (usedSurfaceArea > maxOccupyArea) {
						maxOccupyArea = usedSurfaceArea;
						nonImproved = 1;
					}
					if (usedSurfaceArea >= maxOccupyArea) {
						itemSeq = tempSeq;
					}
				}
			}

		}

		return false;

	}

	bool MaxRectsBinPack::myHeuristicPack(vector<rbp::RectSize> rectSequence) {
		//RandomLS procedure:
		vector<RectSize> itemSeq = rectSequence;
		vector<RectSize> inputSeq;
		vector<Rect> outputSeq;
		int usedSurfaceArea = 0;
		int maxOccupyArea = 0;
		int iterNum = rectSequence.size();

		for (int sortRule = 1; sortRule <= 3; ++sortRule) {
			switch (sortRule) {
			case 1:sort(itemSeq.begin(), itemSeq.end(), ComparatorArea()); break;
			case 2:sort(itemSeq.begin(), itemSeq.end(), ComparatorWidth()); break;
			case 3:sort(itemSeq.begin(), itemSeq.end(), ComparatorHeight()); break;
			}
			//初次调用装箱算法
			inputSeq = itemSeq;
			outputSeq.clear();
			this->compHeuristicInsert(inputSeq, outputSeq);
			if (inputSeq.size() == 0)  return true;
			else {
				usedSurfaceArea = 0;
				for (int i = 0; i < outputSeq.size(); ++i)
					usedSurfaceArea += outputSeq[i].width * outputSeq[i].height;
				maxOccupyArea = usedSurfaceArea;
			}

			for (int nonImproved = 1; nonImproved <= iterNum; ++nonImproved) {
				inputSeq = itemSeq;
				//在itemSeq中随机更换两个item的位置
				//要取得[a, b]的随机整数，使用(rand() % (b - a + 1)) + a;
				int pos1 = std::rand() % itemSeq.size();
				int pos2 = std::rand() % itemSeq.size();
				swap(inputSeq[pos1], inputSeq[pos2]);
				vector<RectSize> tempSeq = inputSeq; //保存这个input sequence
				outputSeq.clear();
				this->compHeuristicInsert(inputSeq, outputSeq);
				if (inputSeq.size() == 0)
					return true;
				else {
					usedSurfaceArea = 0;//必须先清零，否则出错
					for (int i = 0; i < outputSeq.size(); ++i) {
						usedSurfaceArea += outputSeq[i].width * outputSeq[i].height;
					}
					if (usedSurfaceArea > maxOccupyArea) {
						maxOccupyArea = usedSurfaceArea;
						nonImproved = 1;
						itemSeq = tempSeq;
					}
				}
			}

		}

		return false;

	}


	void MaxRectsBinPack::biasedRandomisedInsert(std::vector<RectSize>& rects, std::vector<Rect>& dst, FreeRectChoiceHeuristic method)
	{
		dst.clear();
		while (rects.size() > 0)
		{
			Rect bestNode;
			vector<RectScoreResult> rectScoreSequence;
			RectScoreResult goalRect;
	
			//为每一个rectangle找到其最佳得分对应的摆放位置，相关信息储存在goalRect中
	
			for (int i = 0; i < rects.size(); ++i)
			{
				int score1;
				int score2;
				RectScoreResult currrentRectScore;
				//给当前rectangle打分，将其最佳结果存在RectScoreResult中，如果能放的进去对应的分值已转化为负数，若不能放进去，则分值为极大值
	            //如果已经Cannot fit the current rectangle，则返回的newNode的width和height均为0，并且score1和score2均为极大值
	
				Rect newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);
				currrentRectScore.newNode = newNode;
				//currrentRectScore.newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);
				currrentRectScore.score1 = score1;
				currrentRectScore.score2 = score2;
				currrentRectScore.rectIndex = i;
				rectScoreSequence.push_back(currrentRectScore);
			}
	
			sort(rectScoreSequence.begin(), rectScoreSequence.end(), ComparatorRectScore());
	
			// if there exists one rectangle which cannot be loaded to the bin, return
			if (rectScoreSequence.back().newNode.height == 0)
				return;
	
			//now,we get the goal Rectangle by the baised-randomized method:
			std::uniform_real_distribution<double> uniformDist1(0.06, 0.23);
			std::uniform_real_distribution<double> uniformDist2(0, 1);
			static default_random_engine defaultEngineBinPack;
	
			double beta = uniformDist1(defaultEngineBinPack);
			double randomValue = uniformDist2(defaultEngineBinPack);
			int n = 0;
			double cumulativeProbability = 0;
			for (int i = 0; i < rectScoreSequence.size(); ++i) {
				double rectProbability = beta * pow((1 - beta), n);
				cumulativeProbability += rectProbability;
				if (randomValue < cumulativeProbability) {
					goalRect = rectScoreSequence[i];
					break;
				}
				else {
					n = n + 1;
				}
			}
			//当生成的随机数在比rectScoreSequence.size()个几何概率的累积和还大时，随机选一个
			if (randomValue >= cumulativeProbability) {
				//uniform_int_distribution的随机数的范围是[  ]
				std::uniform_int_distribution<int> uniformDist3(0, rectScoreSequence.size() - 1);
				int k = uniformDist3(defaultEngineBinPack);
				goalRect = rectScoreSequence[k];
			}
	
	
			PlaceRect(goalRect.newNode);//放置goalRect对应的那个rectangle
			dst.push_back(goalRect.newNode);
			rects.erase(rects.begin() + goalRect.rectIndex);
		}
	
	}


	//为当前Rect找到最佳的item,返回的是该Rect和item的组合(代表一个placement)
	Rect MaxRectsBinPack::findBestItem(std::vector<RectSize>& items, Rect& currentRect, int& bestPos) {
		Rect bestNode;
		int bestScore = std::numeric_limits<int>::max();
		bestPos = std::numeric_limits<int>::max();
		Rect placement;

		//对每个items[i]进行测试，看能否装到currentRect中，若能，则计算出相应的更新之后的free Rects 的数量
        //然后进行回滚操作，还原freeRectangles和usedRectangles.
		std::vector<Rect> tempFreeRectangles;
		std::vector<Rect> tempUsedRectangles;
		tempFreeRectangles = freeRectangles;    //先保存，后还原
		tempUsedRectangles = usedRectangles;

		for (int i = 0; i < items.size(); ++i) {
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (currentRect.width >= items[i].width && currentRect.height >= items[i].height) {
				placement.x = currentRect.x;
				placement.y = currentRect.y;
				placement.width = items[i].width;
				placement.height = items[i].height;
				PlaceRect(placement);
				if (freeRectangles.size() < bestScore) {
					bestScore = freeRectangles.size();
					bestPos = i;
					bestNode = placement;
				}
				//还原 freeRectangles、usedRectangles
				freeRectangles = tempFreeRectangles;
				usedRectangles = tempUsedRectangles;
			}

			// Try to place the rectangle in rotated way.
			if (binAllowFlip && currentRect.width >= items[i].height && currentRect.height >= items[i].width) {
				placement.x = currentRect.x;
				placement.y = currentRect.y;
				placement.width = items[i].height;
				placement.height = items[i].width;
				PlaceRect(placement);
				if (freeRectangles.size() < bestScore) {
					bestScore = freeRectangles.size();
					bestPos = i;
					bestNode = placement;
				}
				//还原 freeRectangles、usedRectangles
				freeRectangles = tempFreeRectangles;
				usedRectangles = tempUsedRectangles;
			}
		}
		//若当前Rect不能放置任何items，则返回的bestNode.height和bestNode.width均为0.
		return bestNode;
	}

	void MaxRectsBinPack::heuristicInsert(std::vector<RectSize>& rects, std::vector<Rect>& dst) {
		//调用heuristicInsert()之前需要进行初始化
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		Rect M;
		M = freeRectangles.front();
		int bestPos = -1;  // bestPos record the index of the best matched item in the rects.
		RectSize firstItem = rects.front();
		Rect placement;
		placement.x = M.x;
		placement.y = M.y;
		placement.height = firstItem.height;
		placement.width = firstItem.width;

		PlaceRect(placement);
		dst.push_back(placement);
		rects.erase(rects.begin() + 0);

		while (freeRectangles.size() != 0 && rects.size() != 0) {
			sort(freeRectangles.begin(), freeRectangles.end(), ComparatorFreeRect());
			M = freeRectangles.front();
			placement = findBestItem(rects, M, bestPos);
			if (placement.height != 0) {  //already find the best fit item for the current M.
				PlaceRect(placement);
				dst.push_back(placement);
				rects.erase(rects.begin() + bestPos);
			}
			else {// if doesn't find the best fit item, then remove M from the freeRectangles.
				freeRectangles.erase(freeRectangles.begin() + 0);
			}
		}

	}


	//采用MTP算法：为当前Rect找到最佳的item,返回的是该Rect和item的组合(代表一个placement)
	Rect MaxRectsBinPack::findSuitItemMTP(std::vector<RectSize>& items, Rect& currentRect, int& bestPos) {
		Rect bestNode;
		bestPos = std::numeric_limits<int>::max();

		int bestContactScore = -1;

		for (int i = 0; i < items.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (currentRect.width >= items[i].width && currentRect.height >= items[i].height)
			{
				int score = ContactPointScoreNode(currentRect.x, currentRect.y, items[i].width, items[i].height);
				if (score > bestContactScore)
				{
					bestNode.x = currentRect.x;
					bestNode.y = currentRect.y;
					bestNode.width = items[i].width;
					bestNode.height = items[i].height;
					bestContactScore = score;
					bestPos = i;
				}
			}
			// Try to place the rectangle in rotated loading method.
			if (binAllowFlip && currentRect.width >= items[i].height && currentRect.height >= items[i].width)
			{
				int score = ContactPointScoreNode(currentRect.x, currentRect.y, items[i].height, items[i].width);
				if (score > bestContactScore)
				{
					bestNode.x = currentRect.x;
					bestNode.y = currentRect.y;
					bestNode.width = items[i].height;
					bestNode.height = items[i].width;
					bestContactScore = score;
					bestPos = i;
				}
			}
		}

		//若当前Rect不能放置任何items，则返回的bestNode.height和bestNode.width均为0.
		return bestNode;

	}

	//采用BAF算法：为当前Rect找到最佳的item,返回的是该Rect和item的组合(代表一个placement)
	Rect MaxRectsBinPack::findSuitItemBAF(std::vector<RectSize>& items, Rect& currentRect, int& bestPos) {
		Rect bestNode;
		//score 1
		int bestAreaFit = std::numeric_limits<int>::max();
		//score 2
		int bestShortSideFit = std::numeric_limits<int>::max();

		for (int i = 0; i < items.size(); ++i)
		{
			int areaFit = currentRect.width * currentRect.height - items[i].width * items[i].height;

			// Try to place the rectangle in upright (non-flipped) orientation.
			if (currentRect.width >= items[i].width && currentRect.height >= items[i].height)
			{
				int leftoverHoriz = abs(currentRect.width - items[i].width);
				int leftoverVert = abs(currentRect.height - items[i].height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = currentRect.x;
					bestNode.y = currentRect.y;
					bestNode.width = items[i].width;
					bestNode.height = items[i].height;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
					bestPos = i;
				}
			}

			if (binAllowFlip && currentRect.width >= items[i].height && currentRect.height >= items[i].width)
			{
				int leftoverHoriz = abs(currentRect.width - items[i].height);
				int leftoverVert = abs(currentRect.height - items[i].width);
				int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = currentRect.x;
					bestNode.y = currentRect.y;
					bestNode.width = items[i].height;
					bestNode.height = items[i].width;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
					bestPos = i;
				}
			}
		}

		//若当前Rect不能放置任何items，则返回的bestNode.height和bestNode.width均为0.
		return bestNode;

	}
	
	//为每个bottom left free Rect, 选定相应的最佳item来装填。
	void MaxRectsBinPack::heuristicInsertRectFirst(std::vector<RectSize>& rects, std::vector<Rect>& dst, FreeRectChoiceHeuristic method) {
		//调用heuristicInsertRectFirst()之前需要进行初始化

		Rect M;              // M indicates the most bottom left free Rect in the freeList.
		int bestPos = -1;// bestPos record the index of the best matched item in the rects.

		// pack the first item into the bin.
		// 被初始化后，freeRectangles的唯一一个元素就是最大的长方形（大箱子）.
		M = freeRectangles.front();
		RectSize firstItem = rects.front();
		Rect placement;
		placement.x = M.x;
		placement.y = M.y;
		placement.height = firstItem.height;
		placement.width = firstItem.width;
		PlaceRect(placement);
		dst.push_back(placement);
		rects.erase(rects.begin() + 0);

		while (freeRectangles.size() > 0 && rects.size() > 0) {
			sort(freeRectangles.begin(), freeRectangles.end(), ComparatorFreeRect());
			M = freeRectangles.front();
			switch (method) {
			case RectContactPointRule:        
				placement = findSuitItemMTP(rects, M, bestPos); break;
			case RectBestAreaFit:
				placement = findSuitItemBAF(rects, M, bestPos); break;
			}
			if (placement.height != 0) {  //can find the best fit item for the current M.
				PlaceRect(placement);
				dst.push_back(placement);
				rects.erase(rects.begin() + bestPos);
			}
			else { // if doesn't find the best fit item, then remove M from the freeRectangles.
				freeRectangles.erase(freeRectangles.begin() + 0);
			}
		}

	}

	//根据item的输入序列，依次为每个item选择最合适它的 free Rect.
	void MaxRectsBinPack::heuristicInsertItemFirst(std::vector<RectSize>& rects, std::vector<Rect>& dst, FreeRectChoiceHeuristic method) {
		//调用heuristicInsertItemFirst()之前需要进行初始化
		std::vector<RectSize> unPackedRects;
		while (rects.size()>0){
			RectSize currentItem = rects.front();
			int score1 = std::numeric_limits<int>::max();
			int score2 = std::numeric_limits<int>::max();
			Rect bestPlacement = ScoreRect(currentItem.width, currentItem.height, method, score1, score2);
			if (bestPlacement.height == 0) {
				unPackedRects.push_back(currentItem);
				rects.erase(rects.begin() + 0);
			}
			else {
				PlaceRect(bestPlacement);
				dst.push_back(bestPlacement);
				rects.erase(rects.begin() + 0);
			}
		}

		rects = unPackedRects;
	}

	//当时用该函数来装载时，注意，最终的修改后的实参rects可能会与rbp::MaxRectsBinPack::usedRectangles不一样
	//这是因为该函数内使用了多个装载子函数，但通过参数返回的是其中最佳的结果
	void MaxRectsBinPack::compHeuristicInsert(std::vector<RectSize>& rects, std::vector<Rect>& dst) {

		std::vector<RectSize> inputSeq;
		std::vector<Rect> outputSeq;
		std::vector<RectSize> savedInputSeq;
		std::vector<Rect> savedOutputSeq;
		int pakedArea = 0;

		// 1th method.
		inputSeq = rects;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);//主要是对箱子中的usedRectangles和freeRectangles进行处理
		//调用此函数，如果无法装箱成功，this->freeRectangles肯定为空，因为在装的时候都会被删完了。
		this->heuristicInsertRectFirst(inputSeq, outputSeq, FreeRectChoiceHeuristic::RectContactPointRule);
		if (inputSeq.size() == 0) {
			rects = inputSeq;
			dst = outputSeq;
			return;
		}
		else {
			int curerntPackedArea = this->calculatePackedNumArea().first;
			if (curerntPackedArea > pakedArea) {
				pakedArea = curerntPackedArea;
				savedInputSeq = inputSeq;
				savedOutputSeq = outputSeq;
			}
		}

		// 2th method.
		inputSeq = rects;
		outputSeq.clear();
		this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		this->heuristicInsertRectFirst(inputSeq, outputSeq, FreeRectChoiceHeuristic::RectBestAreaFit);
		if (inputSeq.size() == 0) {
			rects = inputSeq;
			dst = outputSeq;
			return;
		}
		else {
			int curerntPackedArea = this->calculatePackedNumArea().first;
			if (curerntPackedArea > pakedArea) {
				pakedArea = curerntPackedArea;
				savedInputSeq = inputSeq;
				savedOutputSeq = outputSeq;
			}
		}

		//// 3rd method.
		//inputSeq = rects;
		//outputSeq.clear();
		//this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		//this->heuristicInsertItemFirst(inputSeq, outputSeq, FreeRectChoiceHeuristic::RectContactPointRule);
		//if (inputSeq.size() == 0) {
		//	rects = inputSeq;
		//	dst = outputSeq;
		//	return;
		//}
		//else {
		//	int curerntPackedArea = this->calculatePackedNumArea().first;
		//	if (curerntPackedArea > pakedArea) {
		//		pakedArea = curerntPackedArea;
		//		savedInputSeq = inputSeq;
		//		savedOutputSeq = outputSeq;
		//	}
		//}

		//// 4th method.
		//inputSeq = rects;
		//outputSeq.clear();
		//this->Init(this->binWidth, this->binHeight, this->binAllowFlip);
		//this->heuristicInsertItemFirst(inputSeq, outputSeq, FreeRectChoiceHeuristic::RectBestAreaFit);
		//if (inputSeq.size() == 0) {
		//	rects = inputSeq;
		//	dst = outputSeq;
		//	return;
		//}
		//else {
		//	int curerntPackedArea = this->calculatePackedNumArea().first;
		//	if (curerntPackedArea > pakedArea) {
		//		pakedArea = curerntPackedArea;
		//		savedInputSeq = inputSeq;
		//		savedOutputSeq = outputSeq;
		//	}
		//}

		//if all the packing method fail, then return the most good paking result among them
		rects = savedInputSeq;
		dst = savedOutputSeq;

	}

	void MaxRectsBinPack::PlaceRect(const Rect& node)
	{
		//第一步:将新放置进来的item同每个free Rect进行比对，找到那些会受到影响的Rects,包括放置这个item的那个Rect和与这个item有重叠的Rects,
		//               然后将每个受到影响的free Rects进行分割。
		size_t numRectanglesToProcess = freeRectangles.size();
		for (size_t i = 0; i < numRectanglesToProcess; ++i)
		{
			if (SplitFreeNode(freeRectangles[i], node))  //在SplitFreeNode()函数内会对受影响的freeRect进行分割，并把分割后的rects加入到freeRectangles里面
			{
				freeRectangles.erase(freeRectangles.begin() + i);
				--i;
				--numRectanglesToProcess;
			}
		}

		//第二步：将多余的不必要的free Rects删掉
		PruneFreeList();

		//第三步：将新放进来的item记录在usedRectangles里面
		usedRectangles.push_back(node);
	}

	Rect MaxRectsBinPack::ScoreRect(int width, int height, FreeRectChoiceHeuristic method, int& score1, int& score2) const
	{
		Rect newNode;
		score1 = std::numeric_limits<int>::max();
		score2 = std::numeric_limits<int>::max();
		switch (method)
		{
		case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
		case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
		case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint(width, height, score1);
			score1 = -score1; // Reverse since we are minimizing, but for contact point score bigger is better.
			break;
		case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
		case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
		}

		// Cannot fit the current rectangle. 
		// Especially for NewNodeContactPoint Method, if the current item cannot be packed into any rects, score1 will not transfer to negtive.
		if (newNode.height == 0)
		{
			score1 = std::numeric_limits<int>::max();
			score2 = std::numeric_limits<int>::max();
		}

		return newNode;
	}

	/// Computes the ratio of used surface area.
	float MaxRectsBinPack::Occupancy() const
	{
		unsigned long usedSurfaceArea = 0;
		for (size_t i = 0; i < usedRectangles.size(); ++i)
			usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

		return (float)usedSurfaceArea / (binWidth * binHeight);
	}

	// Computes the total area of used surface.
	int MaxRectsBinPack::occupancyArea() const
	{
		int usedSurfaceArea = 0;
		for (int i = 0; i < usedRectangles.size(); ++i)
			usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

		return usedSurfaceArea;
	}


	Rect MaxRectsBinPack::FindPositionForNewNodeBottomLeft(int width, int height, int& bestY, int& bestX) const
	{
		Rect bestNode;
		memset(&bestNode, 0, sizeof(Rect));

		bestY = std::numeric_limits<int>::max();
		bestX = std::numeric_limits<int>::max();

		for (size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int topSideY = freeRectangles[i].y + height;
				if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestY = topSideY;
					bestX = freeRectangles[i].x;
				}
			}
			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int topSideY = freeRectangles[i].y + width;
				if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestY = topSideY;
					bestX = freeRectangles[i].x;
				}
			}
		}
		return bestNode;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBestShortSideFit(int width, int height,
		int& bestShortSideFit, int& bestLongSideFit) const
	{
		Rect bestNode;
		memset(&bestNode, 0, sizeof(Rect));

		bestShortSideFit = std::numeric_limits<int>::max();
		bestLongSideFit = std::numeric_limits<int>::max();

		for (size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - width);
				int leftoverVert = abs(freeRectangles[i].height - height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);
				int longSideFit = max(leftoverHoriz, leftoverVert);

				if (shortSideFit < bestShortSideFit || (shortSideFit == bestShortSideFit && longSideFit < bestLongSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}

			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int flippedLeftoverHoriz = abs(freeRectangles[i].width - height);
				int flippedLeftoverVert = abs(freeRectangles[i].height - width);
				int flippedShortSideFit = min(flippedLeftoverHoriz, flippedLeftoverVert);
				int flippedLongSideFit = max(flippedLeftoverHoriz, flippedLeftoverVert);

				if (flippedShortSideFit < bestShortSideFit || (flippedShortSideFit == bestShortSideFit && flippedLongSideFit < bestLongSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = flippedShortSideFit;
					bestLongSideFit = flippedLongSideFit;
				}
			}
		}
		return bestNode;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBestLongSideFit(int width, int height,
		int& bestShortSideFit, int& bestLongSideFit) const
	{
		Rect bestNode;
		memset(&bestNode, 0, sizeof(Rect));

		bestShortSideFit = std::numeric_limits<int>::max();
		bestLongSideFit = std::numeric_limits<int>::max();

		for (size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - width);
				int leftoverVert = abs(freeRectangles[i].height - height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);
				int longSideFit = max(leftoverHoriz, leftoverVert);

				if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}

			// Try to place the rectangle in flipped orientation.
			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - height);
				int leftoverVert = abs(freeRectangles[i].height - width);
				int shortSideFit = min(leftoverHoriz, leftoverVert);
				int longSideFit = max(leftoverHoriz, leftoverVert);

				if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}
		}
		return bestNode;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBestAreaFit(int width, int height, int& bestAreaFit, int& bestShortSideFit) const
	{
		Rect bestNode;
		memset(&bestNode, 0, sizeof(Rect)); //initialize

		bestAreaFit = std::numeric_limits<int>::max();
		bestShortSideFit = std::numeric_limits<int>::max();

		for (size_t i = 0; i < freeRectangles.size(); ++i)
		{
			int areaFit = freeRectangles[i].width * freeRectangles[i].height - width * height;

			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - width);
				int leftoverVert = abs(freeRectangles[i].height - height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
				}
			}

			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - height);
				int leftoverVert = abs(freeRectangles[i].height - width);
				int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
				}
			}
		}
		return bestNode;
	}


	/// Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
	int CommonIntervalLength(int i1start, int i1end, int i2start, int i2end)
	{
		if (i1end < i2start || i2end < i1start)
			return 0;
		return min(i1end, i2end) - max(i1start, i2start);
	}

	int MaxRectsBinPack::ContactPointScoreNode(int x, int y, int width, int height) const
	{
		int score = 0;

		if (x == 0 || x + width == binWidth)
			score += height;
		if (y == 0 || y + height == binHeight)
			score += width;

		for (size_t i = 0; i < usedRectangles.size(); ++i)
		{
			if (usedRectangles[i].x == x + width || usedRectangles[i].x + usedRectangles[i].width == x)
				score += CommonIntervalLength(usedRectangles[i].y, usedRectangles[i].y + usedRectangles[i].height, y, y + height);
			if (usedRectangles[i].y == y + height || usedRectangles[i].y + usedRectangles[i].height == y)
				score += CommonIntervalLength(usedRectangles[i].x, usedRectangles[i].x + usedRectangles[i].width, x, x + width);
		}
		return score;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeContactPoint(int width, int height, int& bestContactScore) const
	{
		Rect bestNode;
		memset(&bestNode, 0, sizeof(Rect));

		bestContactScore = -1;

		for (size_t i = 0; i < freeRectangles.size(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, width, height);
				if (score > bestContactScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestContactScore = score;
				}
			}
			// Try to place the rectangle in rotated loading method.
			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, height, width);
				if (score > bestContactScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestContactScore = score;
				}
			}
		}
		return bestNode;
	}

	bool MaxRectsBinPack::SplitFreeNode(Rect freeNode, const Rect& usedNode)
	{
		// Test with SAT if the rectangles even intersect.
		if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x ||
			usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
			return false;

		if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
		{
			// New node at the top side of the used node.
			if (usedNode.y > freeNode.y&& usedNode.y < freeNode.y + freeNode.height)
			{
				Rect newNode = freeNode;
				newNode.height = usedNode.y - newNode.y;
				freeRectangles.push_back(newNode);
			}

			// New node at the bottom side of the used node.
			if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
			{
				Rect newNode = freeNode;
				newNode.y = usedNode.y + usedNode.height;
				newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
				freeRectangles.push_back(newNode);
			}
		}

		if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
		{
			// New node at the left side of the used node.
			if (usedNode.x > freeNode.x&& usedNode.x < freeNode.x + freeNode.width)
			{
				Rect newNode = freeNode;
				newNode.width = usedNode.x - newNode.x;
				freeRectangles.push_back(newNode);
			}

			// New node at the right side of the used node.
			if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
			{
				Rect newNode = freeNode;
				newNode.x = usedNode.x + usedNode.width;
				newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
				freeRectangles.push_back(newNode);
			}
		}

		return true;
	}

	void MaxRectsBinPack::PruneFreeList()
	{
		/*
		///  Would be nice to do something like this, to avoid a Theta(n^2) loop through each pair.
		///  But unfortunately it doesn't quite cut it, since we also want to detect containment.
		///  Perhaps there's another way to do this faster than Theta(n^2).

		if (freeRectangles.size() > 0)
			clb::sort::QuickSort(&freeRectangles[0], freeRectangles.size(), NodeSortCmp);

		for(size_t i = 0; i < freeRectangles.size()-1; ++i)
			if (freeRectangles[i].x == freeRectangles[i+1].x &&
				freeRectangles[i].y == freeRectangles[i+1].y &&
				freeRectangles[i].width == freeRectangles[i+1].width &&
				freeRectangles[i].height == freeRectangles[i+1].height)
			{
				freeRectangles.erase(freeRectangles.begin() + i);
				--i;
			}
		*/

		/// Go through each pair and remove any rectangle that is redundant.
		for (size_t i = 0; i < freeRectangles.size(); ++i)
			for (size_t j = i + 1; j < freeRectangles.size(); ++j)
			{
				if (IsContainedIn(freeRectangles[i], freeRectangles[j]))
				{
					freeRectangles.erase(freeRectangles.begin() + i);
					--i;
					break;
				}
				if (IsContainedIn(freeRectangles[j], freeRectangles[i]))
				{
					freeRectangles.erase(freeRectangles.begin() + j);
					--j;
				}
			}
	}

	unsigned long long MaxRectsBinPack::catchBug = 0;
}
