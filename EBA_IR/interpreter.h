#ifndef _INC_INTERPRETER_
#define _INC_INTERPRETER_

#define IR_STATE_SIZE 64

#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>

#define MAX_THREADS 16
#define MAX_MODULES 10


extern pthread_mutex_t interpreter_lock;

extern void (*eba_states[MAX_THREADS])(void*);
extern void *eba_args[MAX_THREADS];




struct IR_state
{
   uint64_t w_node;
   uint64_t w_thread;
   int64_t* vars;
   int next_line; // TODO: might just use pointer for next line to not have
                  // to keep track of both next line and code buf
   char ***code_buf;
};
typedef struct IR_state IR_state_t;


int confirm_first_word(char **line, char *word);
int match_second_word(char **line, char *word);
int parse_variable(char *word);
void* parse_var_buf(char *word, IR_state_t *IRstate);
void buf_free_if_shorthand(void *buf, char *word);
uint64_t parse_literal(char *word);
void var_errmsg(char *func, int line);
int get_avail_w_thread(void);
int thread_is_avail(int w_thread);


void load_dlhandlers(char *list);
void free_dlhandlers(void);





void run_code(void*);

void * EBA_run(void *arg);

IR_state_t * init_IR_state(void);
void print_IR_state(IR_state_t *IRstate);
void free_IR_state(IR_state_t *IRstate);
void eba_free_IR_state(void*);

#endif
