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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define NUM_SIZE 11
#define BUF_MAXSIZE 65536
// this should be enough
static char buf[BUF_MAXSIZE] = {};
static int len_buf = 0;
static char code_buf[BUF_MAXSIZE + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint32_t choose(uint32_t n) {
  return rand() % n;
}

static void gen_num() {
  uint32_t n = choose(UINT32_MAX);

  //transfer n to a string.
  char num_buffer[NUM_SIZE+1];
  sprintf(num_buffer, "%du", n);
  int len = strlen(num_buffer);
  //avoid the buf overflow.
  int delt = BUF_MAXSIZE - len_buf - 1;
  memcpy(buf + len_buf, num_buffer, delt < len ? delt : len);
  len_buf += delt < len ? delt : len;
}

static void gen(char c) {
  if (len_buf < BUF_MAXSIZE - 1) {
    buf[len_buf] = c;
    len_buf ++;
  }
}

static void gen_rand_op() {
  if (len_buf < BUF_MAXSIZE - 1) {
    switch (choose(4)) {
      case 0: buf[len_buf] = '+'; break;
      case 1: buf[len_buf] = '-'; break;
      case 2: buf[len_buf] = '*'; break;
      default: buf[len_buf] = '/'; break;
    }
    len_buf ++;
  }
}

static void gen_rand_expr() {
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(NULL);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    len_buf = 0;//clear the buf.
    gen_rand_expr();
    buf[len_buf] = '\0';//add the \0 to the end of the buf.

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -w /tmp/.code.c -o /tmp/.expr");//don't warnning the integer overflow.
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result = 0;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    //use the value of ret to filte the /0
    if (ret != -1) {
      printf("%u %s\n", result, buf);
    }
  }
  return 0;
}
