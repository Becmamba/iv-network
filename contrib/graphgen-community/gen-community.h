//
//  gen-community.h
//  graphgen-community
//
//  Created by He Yuxuan on 15/2/10.
//  Copyright (c) 2015年 com.laohyx. All rights reserved.
//

#ifndef __graphgen_community__gen_community__
#define __graphgen_community__gen_community__

#include <stdio.h>
#include <bitset>
#include "Snap.h"
#include "PAPool.h"

namespace GenComm {
  using namespace TSnap;

  class GeneratorInterface {
    int timeStep;
    int interval;
    int interestsChangeInterval;
  public:
    GeneratorInterface() : timeStep(0), interval(0) {};
    void run();
    void setInterval(int itvl) {interval = itvl;};
    void setInterestsChangeInterval(int citvl){interestsChangeInterval = citvl;};
    int getTimeStep() {return timeStep;};
    virtual void onBegin() = 0;
    virtual void onEnd() = 0;
    virtual bool onEachTimeStep(int,int) = 0;
    virtual void onEachInterval(int, int) = 0;
  };
  
}

#endif /* defined(__graphgen_community__gen_community__) */
