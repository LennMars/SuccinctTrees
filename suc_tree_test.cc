#include "suc_tree.h"
#include "climits"
#include "cstdlib"
#include "sys/time.h"

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

void test_correctness(int size, float deepness, int index, int diff) {
  cs::bitVector t;
  make_random_tree(size, deepness, t);
  suc_tree tr(t);
  tr.print_nodes();
  tr.print_bits();
  if (index < 0) {
    for (uint i = 0; i < size * 2; ++i){
      for (int d = -1; d <= 1; ++d) {
        uint fast = tr.fwd_search(i, d);
        uint naive = tr.fwd_search_naive(i, d);
        if (fast != naive) {
          printf("i, d : %d, %d\n", i, d);
          printf("fast, naive : %d, %d\n", fast, naive);
        }
      }
    }
  } else {
    printf("i, d : %d, %d\n", index, diff);
    uint naive = tr.fwd_search_naive(index, diff);
    printf("naive : %d, ", naive);
    uint fast = tr.fwd_search(index, diff);
    printf("fast : %d\n", fast);
  }
}

void test_speed_basic(int size, float deepness) {
  cs::bitVector t;
  make_random_tree(size, deepness, t);
  suc_tree tr(t);
  timeval start, end;
  gettimeofday(&start, NULL);
  for (uint i = 0; i < size * 2; ++i){
    uint fast = tr.fwd_search(i, 0);
  }
  gettimeofday(&end, NULL);
  float passed = (float)(end.tv_sec - start.tv_sec) * 1000000 + (float)(end.tv_usec - start.tv_usec);
  printf("took %f usec per fwd_search\n", passed / size / 2);
}

int main(int argc, char* argv[]) {
  int test = 0;
  int size = 100;
  float deepness = 0.5;
  int index = -1;
  int diff = 0;
  if (argc > 2) {
    test = atoi(argv[1]);
  }
  if (argc > 3) {
    size = atoi(argv[2]);
    deepness = atof(argv[3]);
  }
  if (argc > 5) {
    index = atoi(argv[4]);
    diff = atoi(argv[5]);
  }
  switch(test) {
  case 0 :
    test_correctness(size, deepness, index, diff);
    break;
  case 1 :
    test_speed_basic(size, deepness);
    break;
  case 2 :
    test1();
    break;
  }
  return 0;
}
