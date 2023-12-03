/***********************************************************************************  *
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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static word_t dtoh(char c) {
	switch(c) {
		case 'a': case 'A':
			return 10;
		case 'b': case 'B':
			return 11;
		case 'c': case 'C':
			return 12;
		case 'd': case 'D':
			return 13;
		case 'e': case 'E':
			return 14;
		case 'f': case 'F':
			return 15;
		default:
			return (word_t)(c - '0');
	}
}
word_t ahtoi(const char *str) {
	int len = strlen(str);
	word_t res = 0;
	word_t base = 1;
	int i = len - 1;
	while (str[i] != 'x') {
		res += dtoh(str[i]) * base;
		base *= 16;
		i --;
	}
	return res;
}

static int cmd_x(char *args) {
	//get the string of N
	char *n_str = strtok(NULL, " ");
	//get the number
	int n = atoi(n_str);
	//get the string of the start of the memory
	char *addr_str = args + strlen(n_str) + 1;
	//get the address
	word_t addr = ahtoi(addr_str);
	
	int i;
	for (i = 0; i < n; ++i) {
		word_t content = vaddr_read(addr, 4);
		addr += 4;
		printf("%s+%d: " FMT_WORD "\n", addr_str, i*4, content);
	}
	return 0;
}

static int cmd_w(char *args) {
  //create a new watchpoint.
  new_wp(args);
  printf("Create a new watchpoint for expr: %s\n", args);
  return 0;
}

static int cmd_d(char *args) {
  char *number_str = strtok(NULL, " ");
  int number = atoi(number_str);
  free_wp(number);
  printf("Delete No.%d watchpoint.\n", number);
  return 0;
}
static int cmd_p(char *args) {
  bool success;
  uint32_t result = expr(args, &success);
  assert(success);
  printf("the value is: %u\n", result);
	return 0;
}

static int cmd_info(char *args) {
	if (strcmp(args, "r") == 0) {
		isa_reg_display();
	} else if (strcmp(args, "w") == 0) {
    display_wp();
	} else {
    panic("Bad option for info!\n");
	}
	return 0;
}

static int cmd_si(char *args) {
	int n = args == NULL ? 1 : atoi(args);
	cpu_exec(n);
	return 0;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q }, 
  { "si", "Step n instructions, n is the number of the step", cmd_si },
  { "info", "print the state of the process: r--reg:w--watchpoint", cmd_info },
  { "x", "examine the content of the memory, 32bits every", cmd_x },
  { "p", "compute the value of the expression", cmd_p },
  { "w", "create a watch point for a expression", cmd_w },
  { "d", "delete No.n watch point", cmd_d },
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
