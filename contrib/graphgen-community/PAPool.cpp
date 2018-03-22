//
//  PAPool.cpp
//  
//
//  Created by He Yuxuan on 15/6/16.
//
//

#include "PAPool.h"

void TreeBitSet::setBit(int pos) {
  checkPos(pos);
  if (bs.test(pos))
    return;
  bs.set(pos);
  // update heap
  int segId = pos / SEGMENT_LEN;
  int ptr = segId + SEG_NUM;
  while (ptr > 0) {
    heap[ptr]++;
    ptr = ptr >> 1;
  }
}
void TreeBitSet::resetBit(int pos) {
  checkPos(pos);
  if (!bs.test(pos))
    return;
  bs.reset(pos);
  // update heap
  int segId = pos / SEGMENT_LEN;
  int ptr = segId + SEG_NUM;
  while (ptr > 0) {
    heap[ptr]--;
    ptr = ptr >> 1;
  }
}
bool TreeBitSet::test(int pos) {
  return bs.test(pos);
}
int TreeBitSet::randomPick() {
  int R = random() % setBits() + 1;
  int ptr = 1; // root
  while (ptr < SEG_NUM) {
    if (R > heap[ptr * 2]) { // if R > node.left.value
      R -= heap[ptr * 2];
      ptr = ptr * 2 + 1; // node = node.right
    } else {
      ptr = ptr * 2;  // node = node.left
    }
  }
  int segId = ptr - SEG_NUM;
  
  if (!(segId >= 0 && segId < SEG_NUM))
    printf("ERROR!!\n");
  // find R th set bit in segId
  for (int pos = segId * SEGMENT_LEN; pos < segId * SEGMENT_LEN + SEGMENT_LEN; ++pos) {
    if (bs.test(pos)) {
      R--;
      if (R == 0) {
        return pos;
      }
    }
  }
  printf("Should never get here, randomPick()\n");
  exit(1);
}
