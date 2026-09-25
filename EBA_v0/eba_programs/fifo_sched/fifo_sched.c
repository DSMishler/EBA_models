#include "eba_common.h"
#include "fifo_sched_common.h"

// creates a FIFO scheduler with no interrupts and unlimited wait queue.

// options: will we do A or B?
// A: requires all operations that are called to have a "next" field, which
// this will set to point back to itself
// B: requires EBA_run or some other function that will always run, and the
// next op that it runs will be some kind of global or settable variable.
// Perhaps it is passed to called funcitons
//
// option B is the better option, as option A would result in arbitrarily
// long call depth when implemented using C.



void sched_init(void *args)
{
   // args are:
   // 0: this operation
   // 1: where I will place the scheduler
   fifo_sched_t *fs = malloc(sizeof(fifo_sched_t));

   fs->arg_block_next = calloc(SCHED_BUF_LENGTH, sizeof(void*));
   fs->arg_block_end = fs->arg_block_next;
   fs->next_idx = 0;
   fs->end_idx = 0;
   fs->add_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "add_to_sched");
   fs->last_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "last_entry_in_buf");
   
   void *new_buf_last_entry = init_eba_arg(4);
   set_eba_arg(new_buf_last_entry, 0, fs->last_opl);
   set_eba_arg(new_buf_last_entry, 1, fs);
   set_eba_arg(new_buf_last_entry, 2, fs->arg_block_next);
   set_eba_arg(new_buf_last_entry, 3, NULL);

   fs->arg_block_next[SCHED_BUF_LENGTH-1] = new_buf_last_entry;

   set_eba_arg(args, 1, fs);
}

void last_entry_in_buf(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the arg buf to free
   // 3: the next arg_block_next pointer (might be NULL)
   fifo_sched_t *fs = get_eba_arg(args, 1);

   free(get_eba_arg(args, 2));

   fs->arg_block_next = get_eba_arg(args, 3);

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
   if (fs->arg_block_end[fs->end_idx] == NULL)
   {
      // then all is well. Add it in.
      fs->arg_block_end[fs->end_idx] = add_me;
      fs->end_idx += 1;
   }
   else
   {
      // then instead of adding it in, we'll add it in in a NEW block
      // for error check, we will assert here
      if (fs->end_idx != SCHED_BUF_LENGTH-1)
      {
         fprintf(stderr, "there's been a scheduling error. Add conflict "
                 "should only occur at end of block\n");
         // sched_print(fs);
         exit(1);
      }
      void *last_entry_arg_buf = fs->arg_block_end[fs->end_idx];
      
      void **new_buf = calloc(SCHED_BUF_LENGTH,sizeof(void*));

      
      set_eba_arg(last_entry_arg_buf, 3, new_buf);
      fs->arg_block_end = new_buf;
      fs->arg_block_end[0] = add_me;
      fs->end_idx = 1;

      void *new_buf_last_entry = init_eba_arg(4);
      set_eba_arg(new_buf_last_entry, 0, fs->last_opl);
      set_eba_arg(new_buf_last_entry, 1, fs);
      set_eba_arg(new_buf_last_entry, 2, new_buf);
      set_eba_arg(new_buf_last_entry, 3, NULL);
      fs->arg_block_end[SCHED_BUF_LENGTH-1] = new_buf_last_entry;
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
      if (fs->arg_block_next == NULL)
      {
         opl_destroy(fs->add_opl);
         opl_destroy(fs->last_opl);
         free(fs);
         printf("scheduler exit: success\n");
         return;
      }

      // else:
      void *next_eba_op = fs->arg_block_next[fs->next_idx];
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

