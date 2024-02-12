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
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#define PRIO_MAX 100000
uint32_t eval(int p, int q);
word_t ahtoi(const char *str);
typedef enum {
  TK_NOTYPE = 256, TK_DDIG, TK_HDIG, TK_DEREF,
  TK_ADD, TK_SUB, TK_MUL, TK_DIV, TK_EQ, TK_NEQ, TK_AND,
  TK_REG,

  TK_NULL = 10000

  /* TODO: Add more token types */

} OPTYPE;

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  {" +", TK_NOTYPE},    // spaces
  {"0x[a-fA-F0-9]+", TK_HDIG},//hex digit
  {"[0-9]+u*", TK_DDIG}, 	// dec digit
  {"!=", TK_NEQ}, //not equal
  {"&&", TK_AND}, //logic operator: and
  {"\\+", TK_ADD},         // plus
  {"\\-", TK_SUB},			// sub
  {"\\*", TK_MUL},			// mult
  {"\\/", TK_DIV}, 		// div
  {"\\(", '('}, 		// open parenthesis
  {"\\)", ')'}, 		// close parenthesis
  {"==", TK_EQ},        // equal
  {"\\$(zero|ra|sp|gp|tp|t[0-6]|s[0-9]|s1[0-1]|a[0-7]|pc)", TK_REG}//register

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

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  nr_token = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    regmatch_t pmatch;
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

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
            //Assert(substr_len < 32, "the Token::str have only 32 bytes!");
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


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
	  printf("match failed!\n");
    *success = false;
    return -1;
  }

  int i;
  for (i = 0; i < nr_token; ++i) {
    if (tokens[i].type == TK_MUL && tokens[i-1].type != TK_DDIG && tokens[i-1].type != TK_HDIG && tokens[i-1].type != ')')
      tokens[i].type = TK_DEREF;
  }
  
  *success = true;
  return eval(0, nr_token-1);
}

bool expr_test(char *e, uint32_t *result) {
  if (!make_token(e)) {
	  printf("match failed!\n");
    return false;
  } else {
    printf("Every tokens matched successfully: ");
    *result = eval(0, nr_token-1);
    printf("the value is : %u\n", *result);
	  return true;
  }
}

static bool check_parenthesis(int p, int q) {
  bool res = false;
  uint32_t  stack_top = 0;
  if (tokens[p].type == '(' && tokens[q].type == ')')
    res = true;
  int i;
  for (i = p; i <= q; ++i) {
    if (tokens[i].type == '(') {
      stack_top ++;
    } else if (tokens[i].type == ')') {
      Assert(stack_top != 0, "the left parentheses is not engough!\n");
      stack_top --;
      if (stack_top == 0 && i != q) {
        res = false;
      }
    }
  }
  Assert(stack_top == 0, "the right parentheses is not enough!\n");
  return res;
}

static uint32_t priority(OPTYPE type) {
  switch (type) {
    case TK_AND:
      return 0;
    case TK_EQ:
      return 1;
    case TK_NEQ:
      return 1;
    case TK_ADD:
      return 2;
    case TK_SUB:
      return 2;
    case TK_MUL:
      return 3;
    case TK_DIV:
      return 3;
    default:
      return PRIO_MAX;
  }
}

static void find_main_op(int p, int q, int *ploc, int *ptype) {
  *ploc = -1;
  *ptype = TK_NULL;
  int is_in_parenthesis = 0;
  int i;
  for (i = p; i <= q; ++i) {
    if (tokens[i].type >= TK_ADD && tokens[i].type <= TK_AND) {
      if (priority(tokens[i].type) <= priority(*ptype) && !is_in_parenthesis) {
        *ploc = i;
        *ptype = tokens[i].type;
      }
    } else if (tokens[i].type == '(')
      is_in_parenthesis ++;
    else if (tokens[i].type == ')')
      is_in_parenthesis --;
  }
}

static uint32_t dereference(uint32_t address) {
  return vaddr_read(address, 4); 
}

uint32_t eval(int p, int q) {
  if (p > q) {
    //bad expression.
    panic("Bad expression!!\n");
  } else if (p == q) {
    if (tokens[p].type == TK_DDIG) {
      return atoi(tokens[p].str);
    } else if (tokens[p].type == TK_HDIG) {
      return ahtoi(tokens[p].str);
    } else if (tokens[p].type == TK_REG) {
      bool success;
      uint32_t result = isa_reg_str2val(tokens[p].str+1, &success);
      assert(success);
      return result;
    } else {
      //bad expression.
      panic("Not a digit!\n");
    }
  } else if (check_parenthesis(p, q) == true) {
    return eval(p + 1, q - 1);
  } else {
    int op_position;//TODO:get the position of the main op.
    int op_type;
    find_main_op(p, q, &op_position, &op_type);
    //if the op_position is -1, this expr is a '*<expr>'
    if (op_position == -1)
      return dereference(eval(p+1, q));
    uint32_t val1 = eval(p, op_position - 1);
    uint32_t val2 = eval(op_position + 1, q);
    switch (op_type) {
      case TK_ADD:
        return val1 + val2;
      case TK_SUB:
        return val1 - val2;
      case TK_MUL:
        return val1 * val2;
      case TK_DIV:
        if (val2 == 0) 
          panic("/0 error: %u / %u", val1, val2);
        return val1 / val2;
      case TK_EQ:
       return val1 == val2;
      case TK_NEQ:
       return val1 != val2;
      case TK_AND:
       return val1 && val2;
      default:
        panic("Bad main operator!\n");
    }
  }
}
