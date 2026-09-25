#define SCHED_BUF_LENGTH 32
// scheduler buffer format
// 0: arg_buf
// 1: arg_buf
// 2: arg_buf
// ....
// SCHED_BUF_LENGTH-1: arg_buf that points to code to free the scheduler
// AND to code that sets the scheduler's new buf


#include <stdint.h>

struct fifo_sched
{
   void ** arg_block_next;
   uint64_t next_idx;
   void ** arg_block_end;
   uint64_t end_idx;
   op_loader_t *add_opl;
   op_loader_t *last_opl;
};
typedef struct fifo_sched fifo_sched_t;


void sched_init(void *args);
void last_entry_in_buf(void *args);
void add_to_sched(void *args);
void sched_run(void *args);

// below is for debugging. NOT an EBA arg.
void sched_print(fifo_sched_t *fs);
