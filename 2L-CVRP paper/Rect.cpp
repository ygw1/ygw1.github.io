
#include <utility>
#include "Rect.h"

namespace rbp {

/*
#include "clb/Algorithm/Sort.h"

int CompareRectShortSide(const Rect &a, const Rect &b)
{
	using namespace std;

	int smallerSideA = min(a.width, a.height);
	int smallerSideB = min(b.width, b.height);

	if (smallerSideA != smallerSideB)
		return clb::sort::TriCmp(smallerSideA, smallerSideB);

	// Tie-break on the larger side.
	int largerSideA = max(a.width, a.height);
	int largerSideB = max(b.width, b.height);

	return clb::sort::TriCmp(largerSideA, largerSideB);
}
*/
/*
int NodeSortCmp(const Rect &a, const Rect &b)
{
	if (a.x != b.x)
		return clb::sort::TriCmp(a.x, b.x);
	if (a.y != b.y)
		return clb::sort::TriCmp(a.y, b.y);
	if (a.width != b.width)
		return clb::sort::TriCmp(a.width, b.width);
	return clb::sort::TriCmp(a.height, b.height);
}
*/

bool IsContainedIn(const Rect &a, const Rect &b)
{
	return a.x >= b.x && a.y >= b.y 
		&& a.x+a.width <= b.x+b.width 
		&& a.y+a.height <= b.y+b.height;
}


bool ComparatorRectScore::operator()(const rbp::RectScoreResult& rectScore1, const rbp::RectScoreResult& rectScore2) const {
	//按照升序，分值越小越好，那么就将分值小的排在前面
	return rectScore1.score1 < rectScore2.score1;
}

//如何理解这个双目比较运算符函数呢？它其实就是做个比较，你来制定规则，参数中的左目和右目，到底谁排在左边，谁排在右边
//如果返回的是true，就把左目放在左边；如果返回的是false，则把右目放在左边
bool ComparatorFreeRect::operator()(const rbp::Rect& rect1, const rbp::Rect& rect2)const{
	//按照bottom-left将所有的free Rects进行排序，先比较bottom，再比较left.
	//Y越小越好，Y相同的情况下，X坐标越小越好.
	if (rect1.y < rect2.y)
		return true;
	else if (rect1.y == rect2.y) {
		if (rect1.x < rect2.x)
			return true;
		else
			return false;
	}
	else
		return false;

};


bool ComparatorArea::operator()(const RectSize& item1, const RectSize& item2) const {
	//按照降序
	if (item1.width * item1.height != item2.width * item2.height)
		return item1.width * item1.height > item2.width* item2.height;
	else
		return item1.width > item2.width;
}

bool ComparatorWidth::operator()(const RectSize& item1, const RectSize& item2) const {
	//按照降序
	if (item1.width != item2.width)
		return item1.width > item2.width;
	else
		return item1.height > item2.height;
}

bool ComparatorHeight::operator()(const RectSize& item1, const RectSize& item2) const {
	//按照降序
	if (item1.height != item2.height)
		return item1.height > item2.height;
	else
		return item1.width > item2.width;

}



}