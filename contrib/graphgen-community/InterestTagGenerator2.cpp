//
//  InterestTagGenerator2.cpp
//  
//
//  Created by He Yuxuan on 15/6/16.
//
//

#include "InterestTagGenerator2.h"
#include "PAPool.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

namespace GenComm {
  void InterestTagGenerator2::onBegin() {
    GraphPt = PUNGraph::New();
    GraphPt->Reserve(numNodes, NodeOutDeg*numNodes);
    nodeTagWeight.Gen(numNodes, numTags);
    nodeMainNum.Gen(numNodes);
    nodeCommSingle.Gen(numTags);
    nodeCommOverlap.Gen(numTags);
    iv_pool.Gen(numTags);

      // add one node for each community
    for (int x = 0; x < numTags; ++x) {
      int nodeId = GraphPt->AddNode();
      Assert(nodeId == x);
      nodeTagWeight.PutXY(nodeId, x, mainInterestFactor); // nodeId, communityId
      iv_pool[x].setWeight(nodeId, mainInterestFactor);
      nodeMainNum[x] = 1;
      nodeCommSingle[x].Add(x);
      nodeCommOverlap[x].Add(x);
    }
    
      // initialize remaining node' interest vectors
    for (int nodeId = numTags; nodeId < numNodes; ++nodeId) {
      const int N = GraphPt->AddNode();
      Assert(nodeId == N);
      
        // generate one node's IV
      TFltV nodeTagV = calcNodeTagVector(N, mainInterestFactor, nodeMainNum);
      // XXX r is set to linear dist
      double rand = TFlt::Rnd.GetUniDev(); // rand(0,1), r in paper
      int flag = 0;
      for (int i = 0; i < numTags; ++i) {
        if(nodeTagV[i].Val != 1) {
          nodeCommOverlap[i].Add(nodeId);
        }
        if(nodeTagV[i].Val != 1 && flag == 0)
        {
          nodeCommSingle[i].Add(N);
          flag++;
        }
        nodeTagWeight.PutXY(N, i, nodeTagV[i] * rand); // nodeTagV * rand
      }
    }
    //    for (int nodeId = 0; nodeId < numNodes; ++nodeId) {
    //      for (int y = 0; y < numTags; ++y) {
    //        printf("%f ", nodeTagWeight.At(nodeId, y).Val);
    //      }
    //      printf("\n");
    //    }
    
    nodeId = 1;
    
  };
  
  void InterestTagGenerator2::onEnd() {
    SaveEdgeList(GraphPt, "output_final.txt");
    // save num main list
    FILE *F = fopen("output_main_num.txt", "wt");
    for (int i = 0; i < numNodes; ++i) {
      fprintf(F, "%d\t%d\n", i, nodeMainNum[i].Val);
    }
    fclose(F);
    // save communities, single belonging
    F = fopen("output_comm_single.txt", "wt");
    fprintf(F, "Modularity: %f\n", TSnap::GetModularity(GraphPt, nodeCommSingle));
    fprintf(F, "Average ClustCf: %f\n", TSnap::GetClustCf(GraphPt));
    for (int i = 0; i < numTags; ++i) {
      for (int j = 0; j < nodeCommSingle[i].Len(); j++){
        fprintf(F, "%d\t%d\n", i, nodeCommSingle[i][j].Val);
      }
    }
    fclose(F);
    
    // save communities, with overlapping nodes
    F = fopen("output_comm_overlap.txt", "wt");
    for (int i = 0; i < numTags; ++i) {
      for (int j = 0; j < nodeCommOverlap[i].Len(); j++){
        fprintf(F, "%d ", nodeCommOverlap[i][j].Val);
      }
      fprintf(F, "\n");
    }
    fclose(F);
    
    TIntPrV degCnt = TIntPrV();
    TSnap::GetDegCnt(GraphPt, degCnt);
    F = fopen("output_degcnt.txt", "wt");
    for(int i = 0; i < degCnt.Len(); i++){
      fprintf(F, "%d\t%f\n", degCnt[i].GetVal1().Val, double(degCnt[i].GetVal2().Val)/numNodes);
    }
    fclose(F);
    
    F = fopen("output_interests.txt", "wt");
    for(int i = 0; i < numNodes; ++i){
      for (int j = 0; j < numTags; ++j) {
        fprintf(F, "%f\t", nodeTagWeight.At(i, j).Val);
      }
      fprintf(F, "\n");
    }
    fclose(F);
    
    
    
    printf("Generating stopped.\n");
  }
  
  bool InterestTagGenerator2::onEachTimeStep(int timeStep,int interestChangeInterval) {
    // choose target, for NodeOutDeg times.
    //    printf("Node id: %d\n", nodeId);
    int retries = 10;
    for (int i = 0; i < NodeOutDeg; ++i) {
      int target = GenInterestTagChooseTarget(nodeId, nodeTagWeight, GraphPt);
      if ((target == nodeId || GraphPt->IsEdge(target, nodeId)) && NodeOutDeg < nodeId && retries > 0) {
        i--;
        retries--;
          printf("nodeId = %d, target = %d failed\n",nodeId,target);
        //          printf("retry\n");
      } else {
        GraphPt->AddEdge(nodeId, target);
          printf("nodeId = %d, target = %d success\n",nodeId,target);
      }
    }
    nodeId++;
    return nodeId < numNodes;
  }
  
  void InterestTagGenerator2::onEachInterval(int timeStep, int interval) {
    printf("time step %d\n", timeStep);
    SaveEdgeList(GraphPt, "output_"+TInt::GetStr(timeStep)+".txt");
  }
  
  
  double InterestTagGenerator2::GaussRand() {
    static double v1, v2, s;
    static int phase = 0;
    double x;
    
    if (0 == phase) {
      do {
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        
        v1 = 2 * u1 - 1;
        v2 = 2 * u2 - 1;
        s = v1 * v1 + v2 * v2;
      } while (1 <= s || 0 == s);
      x = v1 * sqrt(-2 * log(s) / s);
    } else {
      x = v2 * sqrt(-2 * log(s) / s);
    }
    phase = 1 - phase;
    
    return x;
  }
  
  
  int InterestTagGenerator2::chooseByWeight(TFltV weight, int length) {
    TFlt sum_of_weight = 0;
    for (int i = 0; i < length; i++) {
      sum_of_weight += weight[i];
    }
    //        printf("sum of weight: %f\n", sum_of_weight.Val);
    TFlt rnd = TFlt::Rnd.GetUniDev() * sum_of_weight;
    //        printf("rnd: %f\n", rnd.Val);
    for (int i = 0; i < length; i++) {
      if (rnd <= weight[i])
        return i;
      rnd -= weight[i];
    }
    AssertR(0, "should never get here");
    return 0;
  }
  
  int InterestTagGenerator2::GenInterestTagChooseTarget(int nodeId, TFltVV &nodeTagWeight, PUNGraph GraphPt) {
    TFltV weight(nodeId);  // 0 ~ nodeId - 1
    TFltV weights(numTags);
    for (int i = 0; i < numTags; ++i) {
        weights[i] = nodeTagWeight.At(nodeId, i) * iv_pool[i].totalWeight();
    }
    int tagIdx = chooseByWeight(weights, numTags);
    return iv_pool[tagIdx].randomPick();
  }
  
  // 计算tag的总向量和
  TFltV InterestTagGenerator2::calcTagSumVector(int NodeId) {
    TFltV tagSumVector(numTags);
    for (int i = 0; i < numTags; ++i) {
      tagSumVector[i] = iv_pool[i].totalWeight();
    }
    return tagSumVector;
  }
  
  // 计算一个节点的兴趣向量
  TFltV InterestTagGenerator2::calcNodeTagVector(int NodeId, double mainIntrestFactor, TIntV &nodeMainNum) {
    TFltV tagSumV = calcTagSumVector(NodeId);
    //    printf("tagSumV: ");
    for (int i = 0; i < tagSumV.Len(); ++i) {
      tagSumV[i] += 10 * mainIntrestFactor;
      //      printf("%f ", tagSumV[i].Val);
    }
    //    printf("\n");
    int numTags = nodeTagWeight.GetYDim();
    int numMain = 0;
    // numMain possibility as 0.85, 0.1, 0.05
    double rand = TFlt::Rnd.GetUniDev();
    numMain = rand < Pn1 ? 1 : (TFlt::Rnd.GetUniDev() < 0.6667 ? 2 : 3);
      nodeMainNum[NodeId].Val = numMain;
       //printf("%d", numMain);
    TFltV nodeTagV(numTags);
    for (int i = 0; i < numMain; ++i) {
      int mainTagId = chooseByWeight(tagSumV, numTags);
      //      printf("mainTagId: %d\n", mainTagId);
      nodeTagV[mainTagId] += 1 * mainIntrestFactor;
    }
    for (int i = 0; i < numTags; ++i) {
      if (nodeTagV[i].Val == 0) {
        nodeTagV[i] = 1;
      }
    }
    return nodeTagV;
  }
  
}
