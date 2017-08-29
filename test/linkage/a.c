#include <stdio.h>
#include <b.h>

static int a_static_gv;

int main() {
  printf("external GV b: %d", b);
  printf("static GV common: %d", common_gv);
  printf("internal GV: %d", a_static_gv);
}
