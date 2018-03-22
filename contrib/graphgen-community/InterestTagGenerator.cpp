//
//  InterestTagGenerator.cpp
//  graphgen-community
//
//  Created by He Yuxuan on 15/4/5.
//  Copyright (c) 2015年 com.laohyx. All rights reserved.
//

#include "InterestTagGenerator.h"

namespace GenComm {
  void InterestTagGenerator::onBegin() {
    GraphPt = PUNGraph::New();
    GraphPt->Reserve(numNodes, NodeOutDeg*numNodes);
    nodeTagWeight.Gen(numNodes, numTags);
    nodeMainNum.Reserve(numNodes);
    nodeCommSingle.Gen(numTags);
    nodeCommOverlap.Reserve(numTags);
    nodeDeg.Gen(numNodes);

    for (int x = 0; x < numTags; ++x) {
      int nodeId = GraphPt->AddNode();
      Assert(nodeId == x);
      nodeTagWeight.PutXY(nodeId, x, mainInterestFactor);
      nodeMainNum[x] = 1;
      nodeCommSingle[x].Add(x);
      nodeCommOverlap[x].Add(x);
    }

    for (int nodeId = numTags; nodeId < numNodes; ++nodeId) {
      const int N = GraphPt->AddNode();
      Assert(nodeId == N);

      TFltV nodeTagV = calcNodeTagVector(N, mainInterestFactor, nodeMainNum);
      // XXX r is set to linear dist
      double rand = TFlt::Rnd.GetUniDev();
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
        nodeTagWeight.PutXY(N, i, nodeTagV[i] * rand);
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

  void InterestTagGenerator::onEnd() {
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
    
    F = fopen("output_node_deg.txt", "wt");
    for(int i = 0; i < numNodes; ++i){
         fprintf(F, "%d\t%d\n", i, nodeDeg[i].Val);
    }
    fclose(F);

    printf("Q is %f\n",calcQ());
    printf("Generating stopped.\n");
  }

  bool InterestTagGenerator::onEachTimeStep(int timeStep,int interestsChangeInterval) {
      //if nodeid == interestsChangeInterval reset the exist nodes' interests vector
      if(interestsChangeInterval == timeStep){
          //printf("Start to change interests vector, nodeId = %d\n",nodeId);
          for (int InitialNodeId = numTags; InitialNodeId < nodeId; ++InitialNodeId) {
              double ifChange = TFlt::Rnd.GetUniDev();
              if(ifChange<0.95)
                  continue;
              //printf("select nodeId = %d to change  ifChange = %f\n",InitialNodeId,ifChange);
              TFltV nodeTagV = changeNodeTagVector(InitialNodeId, mainInterestFactor);
              // XXX r is set to linear dist
              double rand = TFlt::Rnd.GetUniDev();
              int flag = 0;
              for (int i = 0; i < numTags; ++i) {
                  if(nodeCommOverlap[i].IsIn(InitialNodeId))
                      nodeCommOverlap[i].Del(InitialNodeId);
                  if(nodeTagV[i].Val != 1) {
                      nodeCommOverlap[i].Add(InitialNodeId);
                  if(nodeTagV[i].Val != 1 && flag == 0){
                          nodeCommSingle[i].Add(InitialNodeId);
                          flag++;
                      }
                  }
                  //printf("before nodeId = %d, interestsTag = %d, vec = %f\n",InitialNodeId,i,nodeTagWeight.GetXY(InitialNodeId,i).Val);
                  nodeTagWeight.PutXY(InitialNodeId, i, nodeTagV[i] * rand);
                  //printf("after nodeId = %d, interestsTag = %d, vec = %f\n",InitialNodeId,i,nodeTagWeight.GetXY(InitialNodeId,i).Val);
              }
              reconnect(InitialNodeId);
          }
      }
// choose target, for NodeOutDeg times.
//    printf("Node id: %d\n", nodeId);
    int retries = NodeOutDeg;
      TFltV weight(nodeId);  // 0 ~ nodeId - 1
      // calculate the weight, according to mutual tag weight and node's degree.
      for (int i = 0; i < nodeId; ++i) {
          weight[i] = 0;
          for (int t = 0, numTags = nodeTagWeight.GetYDim(); t < numTags; ++t) {
              weight[i] += nodeTagWeight.At(i, t) * nodeTagWeight.At(nodeId, t);
          }
          //      printf("w: %f\n", weight[i].Val);
          /// XXX remove deg not
          //weight[i] *= GraphPt->GetNI(i).GetDeg();
      }
    for (int i = 0; i < NodeOutDeg; ++i) {
      int target = chooseByWeight(weight, nodeId);
        if ((target == nodeId || GraphPt->IsEdge(target, nodeId))&& NodeOutDeg < nodeId && retries > 0) {
        i--;
        retries--;
        //printf("nodeId = %d, target = %d failed\n",nodeId,target);
        weight[target] = 0;
//          printf("retry\n");
      } else {
        //printf("nodeId = %d, target = %d success\n",nodeId,target);
        int ret = GraphPt->AddEdge(nodeId, target);
          if (ret == -1) {
              nodeDeg[nodeId].Val++;
              nodeDeg[target].Val++;
          }
          weight[target] = 0;
      }
    }
    nodeId++;
    return nodeId < numNodes;
  }

  void InterestTagGenerator::onEachInterval(int timeStep, int interval) {
    SaveEdgeList(GraphPt, "output_"+TInt::GetStr(timeStep)+".txt");
  }


  double InterestTagGenerator::GaussRand() {
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
    int InterestTagGenerator::PseudoPossionRand(double lambda){
        int k = 0;
        double p = 1.0;
        double l=exp(-lambda);
        while (p>=l)
        {
            double u = TFlt::Rnd.GetUniDev();
            p *= u;
            k++;
        }
        return numTags<k?numTags:k;
    }

  int InterestTagGenerator::chooseByWeight(TFltV weight, int length) {
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
    int InterestTagGenerator::Markov(int NodeId,TFltV weight, int length){
        int originMainInterest = -1;
        double rand = TFlt::Rnd.GetUniDev();
        for(int i = 0;i<numTags;++i){
            if (nodeTagWeight.At(NodeId,i).Val >= 1) {
                originMainInterest = i;
                nodeTagWeight.PutXY(NodeId, i, 0);
            }
        }

        if (originMainInterest>=0) {
          int ret = rand<0.6?originMainInterest:(rand<0.75?originMainInterest+1:(rand<0.9?originMainInterest-1:chooseByWeight(weight, length)));
            if(ret >= numTags)
                return (ret-numTags);
            if(ret<0)
                return ret+numTags;
            return ret;
        }else{
            return chooseByWeight(weight, length);
        }
        
    }
//  int InterestTagGenerator::GenInterestTagChooseTarget(int nodeId, TFltVV &nodeTagWeight, PUNGraph GraphPt,TFltV weight) {
//    TFltV weight(nodeId);  // 0 ~ nodeId - 1
//    // calculate the weight, according to mutual tag weight and node's degree.
//    for (int i = 0; i < nodeId; ++i) {
//      weight[i] = 0;
//      for (int t = 0, numTags = nodeTagWeight.GetYDim(); t < numTags; ++t) {
//        weight[i] += nodeTagWeight.At(i, t) * nodeTagWeight.At(nodeId, t);
//      }
//      //      printf("w: %f\n", weight[i].Val);
//      /// XXX remove deg not
//      //weight[i] *= GraphPt->GetNI(i).GetDeg();
//    }
//    return chooseByWeight(weight, nodeId);
//  }

  TFltV InterestTagGenerator::calcTagSumVector(int NodeId) {
    int numTags = nodeTagWeight.GetYDim();
    TFltV tagSumVector(numTags);
    for (int i = 0; i < tagSumVector.Len(); ++i) {
      tagSumVector.SetVal(i, 0);
    }
    for (int x = 0; x < NodeId; ++x) {
      for (int y = 0; y < numTags; ++y) {
        tagSumVector[y] += nodeTagWeight.At(x, y);
      }
    }
    return tagSumVector;
  }

  TFltV InterestTagGenerator::calcNodeTagVector(int NodeId, double mainIntrestFactor, /* output */TIntV &nodeMainNum) {
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
    //double rand = TFlt::Rnd.GetUniDev();
      numMain = PseudoPossionRand(2);//rand < Pn1 ? 1 : (TFlt::Rnd.GetUniDev() < 0.6667 ? 2 : 3); // Pn1 is On in paper
//    printf("%d", numMain);
      nodeMainNum[NodeId].Val = numMain;
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
    
    TFltV InterestTagGenerator::changeNodeTagVector(int NodeId, double mainIntrestFactor) {
        
        TFltV tagSumV = calcTagSumVector(NodeId);
        //    printf("tagSumV: ");
        for (int i = 0; i < tagSumV.Len(); ++i) {
            tagSumV[i] += 10 * mainIntrestFactor;
        }
        int numTags = nodeTagWeight.GetYDim();
        TFltV nodeTagV(numTags);
        for (int i = 0; i < nodeMainNum[i].Val; ++i) {
            int mainTagId = Markov(NodeId,tagSumV,numTags);//chooseByWeight(tagSumV, numTags);
            nodeTagV[mainTagId] += 1 * mainIntrestFactor;
        }
        for (int i = 0; i < numTags; ++i) {
            if (nodeTagV[i].Val == 0) {
                nodeTagV[i] = 1;
            }
        }
        return nodeTagV;
    }

    void InterestTagGenerator::reconnect(int InitialNodeId){
        int retries = NodeOutDeg;
        TFltV weight(InitialNodeId);  // 0 ~ InitialNodeId - 1
        // calculate the weight, according to mutual tag weight and node's degree.
        for (int i = 0; i < InitialNodeId; ++i) {
            weight[i] = 0;
            for (int t = 0, numTags = nodeTagWeight.GetYDim(); t < numTags; ++t) {
                weight[i] += nodeTagWeight.At(i, t) * nodeTagWeight.At(InitialNodeId, t);
            }
        }
        for (int i = 0; i < NodeOutDeg; ++i) {
            int target = chooseByWeight(weight, InitialNodeId);
            if ((target == InitialNodeId || GraphPt->IsEdge(target, InitialNodeId))&& NodeOutDeg < InitialNodeId && retries > 0) {
                i--;
                retries--;
                //printf("reconnect InitialNodeId = %d, target = %d failed\n",InitialNodeId,target);
                weight[target] = 0;
            } else {
                //printf("reconnect InitialNodeId = %d, target = %d success\n",InitialNodeId,target);
                int ret = GraphPt->AddEdge(InitialNodeId, target);
                if (ret == -1) {
                    nodeDeg[nodeId].Val++;
                    nodeDeg[target].Val++;
                }
                weight[target] = 0;
            }
        }
    }
    double InterestTagGenerator::calcQ(){
        double ret = 0;
        int A = 0;
        for (int i = 0; i<nodeId; ++i) {
            for (int j = i+1; j<=nodeId; ++j) {
                if (!inSameComunity(i,j)) {
                    continue;
                }
                if (GraphPt->IsEdge(i, j)) {
                    A = 1;
                }else{
                    A = 0;
                }
                ret += A - nodeDeg[i]*nodeDeg[j] / (2*GraphPt->GetEdges());
            }
        }
        ret = ret/GraphPt->GetEdges();
        return ret;
    }
    bool InterestTagGenerator::inSameComunity(int i, int j){
        TBoolV isIInterest;
        isIInterest.Gen(numTags);
        for (int index = 0; index < numTags; index ++) {
            if (nodeCommOverlap[index].IsIn(i)&&nodeCommOverlap[index].IsIn(j))
                return true;
        }
        return false;
    }
}
