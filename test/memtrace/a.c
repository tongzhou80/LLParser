
#include <stdio.h>
#include <stdlib.h>
#include "tracer.h"

void main() {
  int i = 3;
  int* p1 = &i;
  printf("p1: %p\n", p1);

  int* p2 = malloc(sizeof(int));
  *p2 = 3;
  printf("p2: %p, *p2: %d\n", p2, *p2);
}
