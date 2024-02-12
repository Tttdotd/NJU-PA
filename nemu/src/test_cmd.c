#include <common.h>
uint32_t expr(char *e);
void test_cmd_p() {
  /* test the p command */
  FILE *fp = fopen("./tools/gen-expr/temp.txt", "r");
  uint32_t answer;
  char e[65536];
  int right_count = 0;
  int error_count = 0;
  uint32_t result = 0;
  while (fscanf(fp, "%u%s", &answer, e) != -1) {
    result = expr(e);
    printf("%s\n", e);
    if (result == answer) {
      right_count ++;
      printf("Success!\n");
    }
    else {
      error_count++;
      printf("failed...\n");
    }
  }
  printf("right_count: %d, error_count: %d\n", right_count, error_count);
}
