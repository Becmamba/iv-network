//
//  InterestTagGenerator2.h
//  
//
//  Created by He Yuxuan on 15/6/16.
//
//

#ifndef ____InterestTagGenerator2__
#define ____InterestTagGenerator2__

#include <stdio.h>
#include "gen-community.h"
#include "PAPool.h"
#include "Snap.h"

namespace GenComm {
  class InterestTagGenerator2 : public GeneratorInterface {
  private:
    int numNodes;
    int NodeOutDeg;
    int numTags;
    double mainInterestFactor;
    double Pn1;
    PUNGraph GraphPt;
    TFltVV nodeTagWeight; // nodeId * tagId, nodeTagWeigh[x] -> node x's interest vector
    TIntV nodeMainNum;    // len: node_num; nodeMainNum[x] node x's number of main interests
    TCnComV nodeCommSingle; // 
    TVec<TIntV> nodeCommOverlap; // len: numTags; recording every community's member list
    int nodeId;
    
    TVec<PAPool> iv_pool; // vector of PA Pool
    
    double GaussRand();
    int chooseByWeight(TFltV weight, int length);
    int GenInterestTagChooseTarget(int nodeId, TFltVV &nodeTagWeight, PUNGraph GraphPt);
    TFltV calcTagSumVector(int NodeId);
    TFltV calcNodeTagVector(int NodeId, double mainIntrestFactor, TIntV &nodeMainNum);
  public:
    InterestTagGenerator2(const int& Nodes, const int& NodeOutDeg, const int& numTags, double mainInterestFactor, double Pn1) :
    numNodes(Nodes), NodeOutDeg(NodeOutDeg), numTags(numTags), mainInterestFactor(mainInterestFactor), Pn1(Pn1) {};
    
    void onBegin();
    void onEnd();
    bool onEachTimeStep(int timeStep,int interestsChangeInterval);
    void onEachInterval(int timeStep, int interval);
  };
  
}



#endif /* defined(____InterestTagGenerator2__) */
