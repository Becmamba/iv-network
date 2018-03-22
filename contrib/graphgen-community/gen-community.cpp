//
//  gen-community.cpp
//  graphgen-community
//
//  Created by He Yuxuan on 15/2/10.
//  Copyright (c) 2015年 com.laohyx. All rights reserved.
//

#include "gen-community.h"
#include <string>
namespace GenComm {

  void GeneratorInterface::run() {
    onBegin();
      int change = 1;
    while (true) {
      bool ret = onEachTimeStep(timeStep,interestsChangeInterval*change);
      if(timeStep == interestsChangeInterval*change){
          change ++;
      }
      if (interval != 0 && timeStep != 0 && timeStep % interval == 0) {
          //printf("Start generate log, timeStep = %d, interval = %d\n",timeStep,interval);
        onEachInterval(timeStep, interval);
      }
      if (!ret) {
        break;
      }
      timeStep++;
    }
    onEnd();
  }

}