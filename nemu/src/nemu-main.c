/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
uint32_t expr(char *e);
void test_p();
int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
  /* test cmd_p */
  //test_p();
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}

void test_p() {
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
      printf("Haha...\n");
    }
  }
  printf("right_count: %d, error_count: %d\n", right_count, error_count);

}
