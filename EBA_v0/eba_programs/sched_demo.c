#include "eba_common.h"
#include "fifo_sched_common.h"
#include <stdint.h>
// creates a FIFO scheduler with no interrupts and unlimited wait queue.
// Uses EBA features to manage its memory during its lifespan

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

// we will need:
// free old fifo sched bufs buffer
// allocate a new fifo sched bufs buffer


void add_to_sched(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to add to the scheduler
}
