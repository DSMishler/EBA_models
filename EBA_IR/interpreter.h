#ifndef _INC_INTERPRETER_
#define _INC_INTERPRETER_

#define IR_STATE_SIZE 64

#include <stdint.h>

struct INVOKE_request
{
   void* code_buf;
   void* arg_buf;
   struct INVOKE_request *next;
};
typedef struct INVOKE_request INVOKE_request_t;

struct IR_state
{
   int64_t* vars;
   int next_line;
   INVOKE_request_t *next_invoke;
};
typedef struct IR_state IR_state_t;

void run_code(INVOKE_request_t *current_invoke);
IR_state_t * init_IR_state(void);
void free_IR_state(IR_state_t *IRstate);

#endif
