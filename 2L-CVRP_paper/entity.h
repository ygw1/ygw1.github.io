#ifndef RECT_H
#define RECT_H

#include <vector>
#include <cassert>
#include <cstdlib>

namespace rbp {

struct RectSize
{
	int width=0;
	int height=0;
	int clientID = 0;
	bool belongsToLinehaul = true;
};

struct Rect
{
	int x=0;
	int y=0;
	int width=0;
	int height=0;
};

struct RectScoreResult
{
	int score1;
	int score2;
	Rect newNode;
	int rectIndex;
};

struct ComparatorRectScore {
	bool operator()(const rbp::RectScoreResult& rectScore1, const rbp::RectScoreResult& rectScore2) const;
};

struct ComparatorFreeRect {
	bool operator()(const rbp::Rect& rect1, const rbp::Rect& rect2)const;
};

struct ComparatorArea {
	bool operator()(const RectSize& item1, const RectSize& item2) const;
};
struct ComparatorWidth {
	bool operator()(const RectSize& item1, const RectSize& item2) const;

};
struct ComparatorHeight {
	bool operator()(const RectSize& item1, const RectSize& item2) const;
};


/// Performs a lexicographic compare on (rect short side, rect long side).
/// @return -1 if the smaller side of a is shorter than the smaller side of b, 1 if the other way around.
///   If they are equal, the larger side length is used as a tie-breaker.
///   If the rectangles are of same size, returns 0.
int CompareRectShortSide(const Rect &a, const Rect &b);

/// Performs a lexicographic compare on (x, y, width, height).
int NodeSortCmp(const Rect &a, const Rect &b);

/// Returns true if a is contained in b.
bool IsContainedIn(const Rect &a, const Rect &b);

class DisjointRectCollection
{
public:
	std::vector<Rect> rects;

	bool Add(const Rect &r)
	{
		// Degenerate rectangles are ignored.
		if (r.width == 0 || r.height == 0)
			return true;

		if (!Disjoint(r))
			return false;
		rects.push_back(r);
		return true;
	}

	void Clear()
	{
		rects.clear();
	}

	bool Disjoint(const Rect &r) const
	{
		// Degenerate rectangles are ignored.
		if (r.width == 0 || r.height == 0)
			return true;

		for(size_t i = 0; i < rects.size(); ++i)
			if (!Disjoint(rects[i], r))
				return false;
		return true;
	}

	static bool Disjoint(const Rect &a, const Rect &b)
	{
		if (a.x + a.width <= b.x ||
			b.x + b.width <= a.x ||
			a.y + a.height <= b.y ||
			b.y + b.height <= a.y)
			return true;
		return false;
	}
};

}



#endif