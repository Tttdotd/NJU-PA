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

extern uint64_t g_nr_guest_inst;
FILE *log_fp = NULL;
FILE *log_ftrace_fp = NULL;
FILE *log_mtrace_fp = NULL;
FILE *log_dtrace_fp = NULL;

void init_log(const char *log_file, const char *log_ftrace_file, 
              const char *log_mtrace_file, const char *log_dtrace_file) {
    log_fp = stdout;
    if (log_file != NULL) {
        FILE *fp = fopen(log_file, "w");
        Assert(fp, "Can not open '%s'", log_file);
        log_fp = fp;
    }
    Log("Log is written to %s", log_file ? log_file : "stdout");
  
    if (log_ftrace_file != NULL) {
        FILE *fp = fopen(log_ftrace_file, "w");
        Assert(fp, "Can not open '%s'", log_ftrace_file);
        log_ftrace_fp = fp;
        Log("Ftrace log is written to %s", log_ftrace_file);
    }

    if (log_mtrace_file != NULL) {
        FILE *fp = fopen(log_mtrace_file, "w");
        Assert(fp, "Can not open '%s'", log_mtrace_file);
        log_mtrace_fp = fp;
        Log("Mtrace log is written to %s", log_ftrace_file);
    }

    if (log_dtrace_file != NULL) {
        FILE *fp = fopen(log_dtrace_file, "w");
        Assert(fp, "Can not open '%s'", log_dtrace_file);
        log_dtrace_fp = fp;
        Log("Dtrace log is written to %s", log_dtrace_file);
    }
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
