#include "eba_common.h"
#include "fifo_sched_common.h"
#include <stdint.h>
// creates a FIFO scheduler with no interrupts and unlimited wait queue.
// Uses EBA features to manage its memory during its lifespan

// options: will we do A or B?
// A: requires all operations that are called to have a "next" field, which
// this will set to point back to itself
// B: requires EBA_run or some other function that will always run, and the
// next op that it runs will be some kind of global or settable variable.
// Perhaps it is passed to called funcitons
//
// option B is the better option, as option A would result in arbitrarily
// long call depth when implemented using C.

#define SCHED_BUF_LENGTH 32
// scheduler buffer format
// 0: arg_buf
// 1: arg_buf
// 2: arg_buf
// ....
// SCHED_BUF_LENGTH-1: arg_buf that points to code to free the scheduler
// AND to code that sets the scheduler's new buf


struct fifo_sched
{
   void ** arg_bufs_buf_next;
   uint64_t next_idx;
   void ** arg_bufs_buf_end;
   uint64_t end_idx;
};
typedef struct fifo_sched fifo_sched_t;


void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("scheduler demo\n");
}

void last_entry_in_buf(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to free
   // 3: the next arg_bufs_buf_next pointer (might be NULL)
}

void add_to_sched(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to add to the scheduler
}
