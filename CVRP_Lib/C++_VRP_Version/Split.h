
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
 // cliSplit[i]对应的客户即为indiv->chromT[i - 1], indiv的chromT是不包含deport结点的，而cliSplit包含了deport结点
  //相比要注意的是，Individual结构的个体引入到split环境中，indiv->chromT结构需要转化为Split环境中的cliSplit结构，cliSplit结构即为拓扑序结构，包含了0结点，
 //而indiv->chromT结构不包含deport结构，所以从indiv->chromT中的每个元素的位置序号都需要加1，才能转化为在cliSplit中的位置序号
  //对于无路径数量限制的情况，只需使用一个一维数组，即potential[0][]即可，对应的元素从potential[0][0]到potential[0][nbClients+1]
 //            对于元素potential[0][j],它表示从拓扑序的0号结点到j结点的最短路程，在运行过程中保持更新
 //            pred结构也只需使用一个一维数组，即pred[0][]，pred[0][j]表示从拓扑序中的0结点到当前的j结点的最短路径中，j结点的直接前驱结点
  //对于有路径数量限制的情况，potential则需要使用二维结构，pred也需要使用二维结构
 std::vector < ClientSplit > cliSplit;
 std::vector < std::vector < double > > potential;  // Potential vector, [nbVehicles + 1][nbClients + 1]
 std::vector < std::vector < int > > pred;  // Indice of the predecessor in an optimal path, [nbVehicles + 1][nbClients + 1]

  // Split for unlimited fleet
  int splitSimple(Individual * indiv);

  // Split for limited fleet
  //P[][]矩阵中的每一列，例如P[k][]，表示的是当允许走k步时，对应的能到达的拓扑序中的结点到结点0的最短距离。
  //显然，P[][]矩阵中的每一列P[k][]中的元素只会从P[k][k]开始，且P[k][k]对应的路线即为: 0,1,2,...,k
  //P[k][i]表示的是，从0结点走k个弧段到i结点的最短距离，而Pred[k][i]则表示，从0结点到达i结点走过了k个弧段的这个最短路径中，i结点的直接前驱结点
  //回溯：如经过查询P矩阵，知道：P[3][9]=6, P[2][6]=3,P[1][3]=0，则从0结点走三步到达9结点的最短路径为：0------3-------6--------9
  //理解这个splitLF算法：
  //
  int splitLF(Individual * indiv);

public:

  // General Split function (tests the unlimited fleet, and only if it does not produce a feasible solution, runs the Split algorithm for limited fleet)
  void generalSplit(Individual * indiv, int nbMaxVehicles);

  // Constructor
  Split(Params * params);

};
#endif
