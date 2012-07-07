#include "suc_tree.h"
#include "climits"
#include "cstdlib"

using std::vector;

int make_random_tree(int size, float deepness, cs::bitVector& t) {
  t.resize(size * 2);
  int d = 0;
  int i = 0;
  for (int opened = 0; opened < size;) {
    float r = (float)rand() / (float)RAND_MAX;
    if (d <= 1 || r < deepness) {
      t.setBit(i, 1);
      ++opened;
      ++d;
      ++i;
    } else {
      t.setBit(i, 0);
      --d;
      ++i;
    }
  }
  for (; d >= 0; --d) {
    t.setBit(i, 0);
    ++i;
  }
  return 0;
}

int test1() {
  const int size = 22;
  cs::bitVector bv(size);
  bool bp[size] = {true, true, false, true, true, true, false, true, false, false, true, false, false, true, true, false, true, false, false, true, false, false};
  for (int i = 0; i < size; ++i) {
    if (bp[i]) bv.setBit(i);
  }

  suc_tree t(bv);
  t.print_nodes();
  for (int i=0; i < size; ++i) {
    printf("%d ", t.block_sum(0, i));
  }
  printf("\n");
  for (int i=0; i < size; ++i) {
    if (bv[i] == 1) {
      printf("( at %d", i);
      printf(" -> %d\n", t.fwd_search(i, 0));
    }
  }

  return 0;
}

void test_correctness(int size, float deepness) {
  cs::bitVector t;
  make_random_tree(size, deepness, t);
  suc_tree tr(t);
  printf("\n");
  for(uint i=0;i < size*2; ++i) {
    printf("%d", t[i] ? 1 : 0);
  }
  printf("\n");
  for(uint i=0; i <= size*2/32; ++i) {
    uint x = t.getBlock(i);
    for(int j=0; j < 32; ++j) {
      if (j != 0 && j % 8 == 0) printf(" ");
      printf("%d", x >> j & 1);
    }
    printf("\n");
  }
  for (uint i = 0; i < size * 2; ++i){
    for (int d = -1; d <= 1; ++d) {
      uint fast = tr.fwd_search(i, d);
      uint naive = tr.fwd_search_naive(i, d);
      if (fast != naive) {
        printf("i, d : %d, %d\n", i, d);
        printf("diff : %d, %d\n", fast, naive);
      }
    }
  }
}

int main(int argc, char* argv[]) {
//   test1();
  test_correctness(10000, 0.5);
  return 0;
}