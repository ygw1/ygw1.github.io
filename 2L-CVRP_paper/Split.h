
#ifndef SPLIT_H
#define SPLIT_H

#include "Params.h"
#include "Individual.h"
#include "MaxRectsBinPack.h"
struct ClientSplit
{
	double demand;
	double serviceTime;
    std::vector<rbp::RectSize> rects;
	double d0_x;
	double dx_0;
	double dnext;
	ClientSplit() : demand(0.), serviceTime(0.), d0_x(0.), dx_0(0.), dnext(0.) {};
};


class Split
{
 private:

 // Problem parameters
 Params * params ;
 int maxVehicles ;
 /* Auxiliary data structures to run the Linear Split algorithm */
 // cliSplit[i]��Ӧ�Ŀͻ���Ϊindiv->chromT[i - 1], indiv��chromT�ǲ�����deport���ģ���cliSplit������deport���
  //���Ҫע����ǣ�Individual�ṹ�ĸ������뵽split�����У�indiv->chromT�ṹ��Ҫת��ΪSplit�����е�cliSplit�ṹ��cliSplit�ṹ��Ϊ������ṹ��������0��㣬
 //��indiv->chromT�ṹ������deport�ṹ�����Դ�indiv->chromT�е�ÿ��Ԫ�ص�λ����Ŷ���Ҫ��1������ת��Ϊ��cliSplit�е�λ�����
  //������·���������Ƶ������ֻ��ʹ��һ��һά���飬��potential[0][]���ɣ���Ӧ��Ԫ�ش�potential[0][0]��potential[0][nbClients+1]
 //            ����Ԫ��potential[0][j],����ʾ���������0�Ž�㵽j�������·�̣������й����б��ָ���
 //            pred�ṹҲֻ��ʹ��һ��һά���飬��pred[0][]��pred[0][j]��ʾ���������е�0��㵽��ǰ��j�������·���У�j����ֱ��ǰ�����
  //������·���������Ƶ������potential����Ҫʹ�ö�ά�ṹ��predҲ��Ҫʹ�ö�ά�ṹ
 std::vector < ClientSplit > cliSplit;
 std::vector < std::vector < double > > potential;  // Potential vector, [nbVehicles + 1][nbClients + 1]
 std::vector < std::vector < int > > pred;  // Indice of the predecessor in an optimal path, [nbVehicles + 1][nbClients + 1]

  // Split for unlimited fleet
  int splitSimple(Individual * indiv);

  // Split for limited fleet
  //P[][]�����е�ÿһ�У�����P[k][]����ʾ���ǵ�������k��ʱ����Ӧ���ܵ�����������еĽ�㵽���0����̾��롣
  //��Ȼ��P[][]�����е�ÿһ��P[k][]�е�Ԫ��ֻ���P[k][k]��ʼ����P[k][k]��Ӧ��·�߼�Ϊ: 0,1,2,...,k
  //P[k][i]��ʾ���ǣ���0�����k�����ε�i������̾��룬��Pred[k][i]���ʾ����0��㵽��i����߹���k�����ε�������·���У�i����ֱ��ǰ�����
  //���ݣ��羭����ѯP����֪����P[3][9]=6, P[2][6]=3,P[1][3]=0�����0�������������9�������·��Ϊ��0------3-------6--------9
  //������splitLF�㷨��
  //
  int splitLF(Individual * indiv);

public:

  // General Split function (tests the unlimited fleet, and only if it does not produce a feasible solution, runs the Split algorithm for limited fleet)
  void generalSplit(Individual * indiv, int nbMaxVehicles);

  // Constructor
  Split(Params * params);

};
#endif
