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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

uint32_t eval(int p, int q);

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DDIG,
  TK_ADD, TK_SUB, TK_MUL, TK_DIV,

  TK_NULL = 10000

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"[0-9]+", TK_DDIG}, 	// dec digit
  {"\\+", TK_ADD},         // plus
  {"\\-", TK_SUB},			// sub
  {"\\*", TK_MUL},			// mult
  {"\\/", TK_DIV}, 		// div
  {"\\(", '('}, 		// open parenthesis
  {"\\)", ')'}, 		// close parenthesis
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
//		    tokens[nr_token].type = rules[i].token_type;
//  		  memcpy(tokens[nr_token].str, substr_start, substr_len);
//  		  Assert(substr_len < 32, "the tokens[i].str have only 32 bytes!");
//    		tokens[nr_token].str[substr_len] = '\0';
//    		nr_token ++;
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          default:
            tokens[nr_token].type = rules[i].token_type;
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            Assert(substr_len < 32, "the Token::str have only 32 bytes!");
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


bool expr(char *e) {
  if (!make_token(e)) {
	  printf("match failed!\n");
    return false;
  } else {
  	/* TODO: Insert codes to evaluate the expression. */
    printf("Every tokens matched successfully: ");
  	int i;
  	for (i = 0; i < nr_token; ++i) {
	    printf("\'%s\',", tokens[i].str);
  	}
	  printf("\n");
    printf("the value is : %d\n", eval(0, nr_token-1));
	  return true;
  }
}

static bool check_parentheses(int p, int q) {
  bool res = false;
  int stack_top = 0;
  if (tokens[p].type == '(' && tokens[q].type == ')')
    res = true;
  int i;
  for (i = p; i <= q; ++i) {
    if (tokens[i].type == '(') {
      stack_top ++;
    } else if (tokens[i].type == ')') {
      assert(stack_top != 0);
      stack_top --;
      if (stack_top == 0 && i != q) {
        res = false;
      }
    }
  }
  assert(stack_top == 0);
  return res;
}

static void main_op(int p, int q, int *ploc, int *ptype) {
  *ploc = -1;
  *ptype = TK_NULL;
  int i;
  bool is_in_pareth = false;
  for (i = p; i <= q; ++i) {
    if (tokens[i].type >= TK_ADD && tokens[i].type <= TK_DIV) {
      if (tokens[i].type < *ptype && !is_in_pareth) {
        *ploc = i;
        *ptype = tokens[i].type;
      }
    } else if (tokens[i].type == '(')
      is_in_pareth = true;
    else if (tokens[i].type == ')')
      is_in_pareth = false;
  }
}
uint32_t eval(int p, int q) {
  if (p > q) {
    //bad expression.
    assert(0);
  } else if (p == q) {
    if (tokens[p].type == TK_DDIG) {
      return atoi(tokens[p].str);
    } else {
      //bad expression.
      assert(0);
    }
  } else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  } else {
    int op_loc;//TODO:get the position of the main op.
    int op_type;
    main_op(p, q, &op_loc, &op_type);
    uint32_t val1 = eval(p, op_loc - 1);
    uint32_t val2 = eval(op_loc + 1, q);
    switch (op_type) {
      case TK_ADD:
        return val1 + val2;
      case TK_SUB:
        return val1 - val2;
      case TK_MUL:
        return val1 * val2;
      case TK_DIV:
        return val1 / val2;
      default:
        assert(0);
    }
  }
}
