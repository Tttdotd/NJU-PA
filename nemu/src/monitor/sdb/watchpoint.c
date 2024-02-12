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

#include "sdb.h"

#define NR_WP 32
#define BUFFER_SIZE 128

typedef enum {
  WATCH, NORMAL
} BREAK_TYPE;

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  struct watchpoint *last;
  char buffer[BUFFER_SIZE];
  uint32_t watch_oldvalue;
  uint32_t time_hit;
} WP;

typedef struct watchpointlist {
  WP* head;
  WP* tail;
  word_t len;
}WPList;

static WP wp_pool[NR_WP] = {};

static WPList list_used = {NULL, NULL, 0};
static WPList list_free = {NULL, NULL, 0};

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].last = (i == 0 ? NULL : &wp_pool[i - 1]);
    wp_pool[i].time_hit = 0;
  }

  list_free.head = wp_pool;
  list_free.tail = &wp_pool[NR_WP-1];
  list_free.len = NR_WP;
}

void new_wp(char *info) {
  //put the first point into the head.
  word_t size_info = strlen(info) + 1;
  assert(size_info <= BUFFER_SIZE);
  if (list_free.head == NULL) {
    printf("No more watch point.\n");
    return;
  }
  memcpy(list_free.head->buffer, info, size_info);
  assert(list_free.head->buffer[size_info-1] == '\0');
  bool success;
  list_free.head->watch_oldvalue = expr(list_free.head->buffer, &success);
  assert(success);
  WP *wp_index = list_free.head;
  list_free.head = list_free.head->next;
  if (list_used.head == NULL) {
    list_used.head = wp_index;
    list_used.head->next = NULL;
    list_used.tail = wp_index;
  } else {
    assert(wp_index != NULL);
    list_used.tail->next = wp_index;
    list_used.tail = list_used.tail->next;
    list_used.tail->next = NULL;
  }
  
  list_used.len ++;
  list_free.len --;
}

void free_wp(int number) {
  assert(number < list_used.len);
  WP* wp_index = list_used.head;
  while (number > 0) {
    wp_index = wp_index->next;
    number --;
  }
  if (wp_index->last == NULL) {
    list_used.head = wp_index->next;
  } else {
    wp_index->last->next = wp_index->next;
  }
  if (list_free.head == NULL) {
    list_free.head = wp_index;
    list_free.tail = wp_index;
    list_free.tail->next = NULL;
  } else {
    list_free.tail->next = wp_index;
    list_free.tail = list_free.tail->next;
    list_free.tail->next = NULL;
  }
  list_free.len ++;
  list_used.len --;
}

void display_wp() {
  WP *wp_index = list_used.head;
  if (wp_index == NULL) {
    printf("No watchpoint.\n");
    return;
  }
  int n = 0;
  while (wp_index != NULL) {
    printf("Num   What\n");
    printf("%d    %s\n", n, wp_index->buffer);
    printf("already hit %u time..\n", wp_index->time_hit);
    wp_index = wp_index->next;
    n++;
  }
}

bool scan_wp() {
  bool flag = false;
  WP *wp_index = list_used.head;
  int n = 0;
  while (wp_index != NULL) {
    bool success;
    uint32_t  watch_newvalue = expr(wp_index->buffer, &success);
    assert(success);
    if (wp_index->watch_oldvalue != watch_newvalue) {
      printf("watchpoint %d: %s\n", n, wp_index->buffer);
      printf("Old value = %x\nNew value = %x\n", wp_index->watch_oldvalue, watch_newvalue);
      wp_index->watch_oldvalue = watch_newvalue;
      flag = true;
    }
    wp_index = wp_index->next;
    n ++;
  }
  return flag;
}
