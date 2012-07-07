#include <stdint.h>
#include <cstdlib>
#include <cstdio>

const int lookup_unit_size = 8;
const int d_length = lookup_unit_size * 2 + 1;
const int i_length = 1 << lookup_unit_size;

int popcnt(uint32_t n) {
  int counter = 0;
  while(n != 0) {
    counter += n & 1;
    n >>= 1;
  }
  return counter;
}

int lookup(uint32_t bits, int d) {
  if (d < -lookup_unit_size || d > lookup_unit_size) {
    return (popcnt(bits) << 4);
  } else {
    int sum = 0;
    int is_found = 0;
    int found_index = 0;
    int popcnt = 0;
    for (int j = 0; j < lookup_unit_size; ++j) {
      int b = bits >> j & 1;
      sum += b ? 1 : -1;
      if (!is_found && sum == d) {
        is_found = 1;
        found_index = j;
      }
      popcnt += b;
    }
    return (popcnt << 4) + (found_index << 1) + is_found;
  }
}

void fill(int** t) {
  for(int d = 0; d < d_length; ++d) {
    for (uint32_t i = 0; i < i_length; ++i) {
      t[d][i] = lookup(i, d - lookup_unit_size);
    }
  }
}

void p(int** t) {
  printf("int table[%d][%d]={\n", d_length, i_length);
  for (int d = 0; d < d_length; ++d) {
    printf("{");
    for (int i = 0; i < i_length - 1; ++i) {
      printf("%d, ", t[d][i]);
    }
    printf("%d}%s\n", t[d][i_length - 1], (d == d_length - 1) ? "" : ",");
  }
  printf("}");
}

int main(int argc, char* argv[]) {
  int** table = (int**)malloc(sizeof(int*) * d_length);
  for(int d = 0; d < d_length; ++d) {
    table[d] = (int*)malloc(sizeof(int) * i_length);
  }
  fill(table);
  p(table);
}
