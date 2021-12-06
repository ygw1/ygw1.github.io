#ifndef CIRCLESECTOR_H
#define CIRCLESECTOR_H

// Simple data structure to represent circle sectors
// Angles are measured in [0,65535] instead of [0,359], in such a way that modulo operations are much faster (since 2^16 = 65536)
//��ƪ���ĵĽǶ���65536�����360�㣬һ����ά�����(x,y)��Ӧ�Ļ���Ϊatan2(y, x)���ڱ����У���ת�����polar angleΪ positive_mod(32768.*atan2(y, x) / PI)
// Credit to Fabian Giesen at "https://web.archive.org/web/20200912191950/https://fgiesen.wordpress.com/2015/09/24/intervals-in-modular-arithmetic/" for useful implementation tips regarding interval overlaps in modular arithmetics 
struct CircleSector
{
	int start;
	int end;

	// Positive modulo 65536
	static int positive_mod(int i)
	{
		// 1) Using the formula positive_mod(n,x) = (n % x + x) % x
		// 2) Moreover, remark that "n % 65536" should be automatically compiled in an optimized form as "n & 0xffff" for faster calculations
		return (i % 65536 + 65536) % 65536;
	}

	// Initialize a circle sector from a single point
	 //pointΪ��ά������Ӧ�ļ��� polarAngle(��65536��ı�׼��)
	void initialize(int point)
	{
		start = point;
		end = point;
	}

	// Tests if a point is enclosed in the circle sector
	 //pointΪ��ά������Ӧ�ļ���polarAngle(��65536��ı�׼��)
	bool isEnclosed(int point)
	{
		return (positive_mod(point - start) <= positive_mod(end - start));
	}

	// Tests overlap of two circle sectors �ж��������������Ƿ��ص�
	static bool overlap(const CircleSector & sector1, const CircleSector & sector2)
	{
		return ((positive_mod(sector2.start - sector1.start) <= positive_mod(sector1.end - sector1.start))
			|| (positive_mod(sector1.start - sector2.start) <= positive_mod(sector2.end - sector2.start)));
	}

	// Extends the circle sector to include an additional point 
	// Done in a "greedy" way, such that the resulting circle sector is the smallest
	 //pointΪ��ά������Ӧ�ļ���polarAngle(��65536��ı�׼��)
	void extend(int point)
	{
		if (!isEnclosed(point))
		{
			if (positive_mod(point - end) <= positive_mod(start - point))
				end = point;
			else
				start = point;
		}
	}
};

#endif
