#include "b.h"

int b = 9;


static int b_static_gv = 1;

void foo() {
  printf("use b static gv: %d", b_static_gv);
  printf("use common gv: %d", common_gv);
}
