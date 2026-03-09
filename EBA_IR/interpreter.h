#ifndef _INC_INTERPRETER_
#define _INC_INTERPRETER_

#define IR_STATE_SIZE 64

#include <stdint.h>


struct IR_state
{
   int64_t* vars;
   int next_line; // TODO: might just use pointer for next line to not have
                  // to keep track of both next line and code buf
   char ***code_buf;
};
typedef struct IR_state IR_state_t;

void run_code(void*);

void EBA_run(void);

IR_state_t * init_IR_state(void);
void print_IR_state(IR_state_t *IRstate);
void free_IR_state(IR_state_t *IRstate);
void eba_free_IR_state(void*);

#endif
