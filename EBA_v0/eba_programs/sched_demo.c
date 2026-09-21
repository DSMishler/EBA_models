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
   op_loader_t *add_opl;
   op_loader_t *last_opl;
};
typedef struct fifo_sched fifo_sched_t;


void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("scheduler demo\n");
   op_loader_t *fs_main_opl = opl_init("eba_programs/sched_demo.so", "sched_run");
   op_loader_t *fs_init_opl = opl_init("eba_programs/sched_demo.so", "sched_init");

   void *fs_main_arg = init_eba_arg(2);
   void *fs_init_arg = init_eba_arg(2);

   set_eba_arg(fs_main_arg, 0, fs_main_opl);
   set_eba_arg(fs_init_arg, 0, fs_init_opl);

   eba_op(fs_init_arg);

   fifo_sched_t *fs = get_eba_arg(fs_init_arg, 1);
   set_eba_arg(fs_main_arg, 1, fs);

   eba_op(fs_main_arg);

   opl_destroy(fs_main_opl);
   opl_destroy(fs_init_opl);
   free_eba_arg(fs_main_arg);
   free_eba_arg(fs_init_arg);
}

void sched_init(void *args)
{
   // args are:
   // 0: this operation
   // 1: where I will place the scheduler
   fifo_sched_t *fs = malloc(sizeof(fifo_sched_t));

   fs->arg_bufs_buf_next = calloc(SCHED_BUF_LENGTH,sizeof(void*));
   fs->arg_bufs_buf_end = fs->arg_bufs_buf_next;
   fs->next_idx = 0;
   fs->end_idx = 0;
   fs->add_opl = opl_init("eba_programs/sched_demo.so", "add_to_sched");
   fs->last_opl = opl_init("eba_programs/sched_demo.so", "last_entry_in_buf");
   
   void *new_buf_last_entry = init_eba_arg(4);
   set_eba_arg(new_buf_last_entry, 0, fs->last_opl);
   set_eba_arg(new_buf_last_entry, 1, fs);
   set_eba_arg(new_buf_last_entry, 2, fs->arg_bufs_buf_next);
   set_eba_arg(new_buf_last_entry, 3, NULL);

   fs->arg_bufs_buf_next[SCHED_BUF_LENGTH-1] = new_buf_last_entry;

   set_eba_arg(args, 1, fs);
}

void last_entry_in_buf(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to free
   // 3: the next arg_bufs_buf_next pointer (might be NULL)
   fifo_sched_t *fs = get_eba_arg(args, 1);

   free(get_eba_arg(args, 2));

   fs->arg_bufs_buf_next = get_eba_arg(args, 3);

   fs->next_idx = 0;

   free_eba_arg(args);
}

void add_to_sched(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to add to the scheduler
   fifo_sched_t *fs = get_eba_arg(args, 1);
   void *add_me = get_eba_arg(args, 2);
   if (fs->arg_bufs_buf_end[fs->end_idx] == NULL)
   {
      // then all is well. Add it in.
      fs->arg_bufs_buf_end[fs->end_idx] = add_me;
   }
   else
   {
      // then instead of adding it in, we'll add it in in a NEW block
      // for error check, we will assert here
      if (fs->end_idx != SCHED_BUF_LENGTH-1)
      {
         fprintf(stderr, "there's been a scheduling error. Add conflict "
                 "should only occur at end of block\n");
         exit(1);
      }
      void *last_entry_arg_buf = fs->arg_bufs_buf_end[fs->end_idx];
      
      void **new_buf = calloc(SCHED_BUF_LENGTH,sizeof(void*));

      
      set_eba_arg(last_entry_arg_buf, 3, new_buf);
      fs->arg_bufs_buf_end = new_buf;
      fs->arg_bufs_buf_end[0] = add_me;
      fs->end_idx = 1;

      void *new_buf_last_entry = init_eba_arg(4);
      set_eba_arg(new_buf_last_entry, 0, fs->last_opl);
      set_eba_arg(new_buf_last_entry, 1, fs);
      set_eba_arg(new_buf_last_entry, 2, new_buf);
      set_eba_arg(new_buf_last_entry, 3, NULL);
      fs->arg_bufs_buf_end[SCHED_BUF_LENGTH-1] = new_buf_last_entry;
   }
   free_eba_arg(args);
}

void sched_run(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   fifo_sched_t *fs = get_eba_arg(args, 1);

   while(1)
   {
      // printf("running with next=%ld\n", fs->next_idx);
      if (fs->arg_bufs_buf_next == NULL)
      {
         opl_destroy(fs->add_opl);
         opl_destroy(fs->last_opl);
         free(fs);
         printf("scheduler exit\n");
         return;
      }

      // else:
      void *next_eba_op = fs->arg_bufs_buf_next[fs->next_idx];
      fs->next_idx += 1;
      if (next_eba_op != NULL)
      {
         eba_op(next_eba_op);
      }
      else
      {
         ; // pass
      }
   }
}
