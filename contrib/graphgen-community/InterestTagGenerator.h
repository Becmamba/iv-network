//
//  InterestTagGenerator.h
//  graphgen-community
//
//  Created by He Yuxuan on 15/4/5.
//  Copyright (c) 2015年 com.laohyx. All rights reserved.
//

#ifndef __graphgen_community__InterestTagGenerator__
#define __graphgen_community__InterestTagGenerator__

#include <stdio.h>
#include "gen-community.h"
namespace GenComm {
  class InterestTagGenerator : public GeneratorInterface {
  private:
    int numNodes;
    int NodeOutDeg;
    int numTags;
    double mainInterestFactor;
    double Pn1;
    PUNGraph GraphPt;
    TFltVV nodeTagWeight;
    TIntV nodeMainNum;
    TCnComV nodeCommSingle;
    TVec<TIntV> nodeCommOverlap;
    int nodeId;
    TIntV nodeDeg;//for calculate Q
    
    double GaussRand();
    int PseudoPossionRand(double lamda);
    int chooseByWeight(TFltV weight, int length);
    int Markov(int NodeId,TFltV weight, int length);
    int GenInterestTagChooseTarget(int nodeId, TFltVV &nodeTagWeight, PUNGraph GraphPt);
    TFltV calcTagSumVector(int NodeId);
    TFltV calcNodeTagVector(int NodeId, double mainIntrestFactor, TIntV &nodeMainNum);
    TFltV changeNodeTagVector(int NodeId, double mainIntrestFactor);
  public:
  InterestTagGenerator(const int& Nodes, const int& NodeOutDeg, const int& numTags, double mainInterestFactor, double Pn1) :
    numNodes(Nodes), NodeOutDeg(NodeOutDeg), numTags(numTags), mainInterestFactor(mainInterestFactor), Pn1(Pn1) {};
    
    void onBegin();
    void onEnd();
    bool onEachTimeStep(int timeStep,int interstsChangeInterval);
    void reconnect(int);
    void onEachInterval(int timeStep, int interval);
    double calcQ();
    bool inSameComunity(int i, int j);
  };

}


#endif /* defined(__graphgen_community__InterestTagGenerator__) */
