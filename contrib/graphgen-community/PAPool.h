//
//  PAPool.h
//  
//
//  Created by He Yuxuan on 15/6/16.
//
//

#ifndef ____PAPool__
#define ____PAPool__

#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include "Snap.h"

#define MAX_ELEMENTS 65536
#define SEGMENT_LEN 64
#define SEG_NUM MAX_ELEMENTS/SEGMENT_LEN
class TreeBitSet {
  std::bitset<65536> bs; // max size
  int size;
  // NOTE: heap starts from 1
  int heap[SEG_NUM * 2];
  void inline checkPos(int pos) {
    if (pos >= size) {
      printf("pos exceeds size: %d, %d\n", pos, size);
      exit(1);
    }
  }
public:
  TreeBitSet(){
    size = MAX_ELEMENTS;
    for (int i = 0 ; i < SEG_NUM * 2; ++i) {
      heap[i] = 0;
    }
  };
  
  void setBit(int pos);
  void resetBit(int pos);
  bool test(int pos);
  int randomPick();
  int inline setBits() {
    return heap[1];
  }
  
};

#define WEIGHT_BIT_LEN 32
class PAPool {
  TreeBitSet tbs[WEIGHT_BIT_LEN];
public:
  void setWeight(int pos, int w) {
    for (int i = 0; i < WEIGHT_BIT_LEN; ++i) {
      if (w == 0) {
        return;
      }
      if (w % 2 == 1) {
        tbs[i].setBit(pos);
      } else {
        tbs[i].resetBit(pos);
      }
      w = w >> 1;
    }
  }
  int getWeight(int pos) {
    int w = 0;
    for (int i = 0; i < WEIGHT_BIT_LEN; ++i) {
      if (tbs[i].test(pos)) {
        w += 1 << i;
      }
    }
    return w;
  }
  
  int totalWeight() {
    int w = 0;
    for (int i = 0; i < WEIGHT_BIT_LEN; ++i) {
      w += (1 << i) * tbs[i].setBits();
    }
    return w;
  }
  
  int chooseByWeight(int weight[], int length) {
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
  
  
  int randomPick() {
    int weights[WEIGHT_BIT_LEN] = {0};
    for (int i = 0; i < WEIGHT_BIT_LEN; ++i) {
      weights[i] = (1 << i) * tbs[i].setBits();
    }
    int idx = 0;
    // random select from weights
    idx = chooseByWeight(weights, WEIGHT_BIT_LEN);
    return tbs[idx].randomPick();
  }
  
};


#endif /* defined(____PAPool__) */
