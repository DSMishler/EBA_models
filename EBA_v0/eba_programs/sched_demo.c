#include "eba_common.h"
#include "fifo_sched_common.h"

// creates a FIFO scheduler with no interrupts and unlimited wait queue.
// Uses EBA features to manage its memory during its lifespan

void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("scheduler demo\n");
   // SCHEDULER INIT

   op_loader_t *fs_main_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "sched_run");
   op_loader_t *fs_init_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "sched_init");

   void *fs_main_arg = init_eba_arg(2);
   void *fs_init_arg = init_eba_arg(2);

   set_eba_arg(fs_main_arg, 0, fs_main_opl);
   set_eba_arg(fs_init_arg, 0, fs_init_opl);

   eba_op(fs_init_arg);

   fifo_sched_t *fs = get_eba_arg(fs_init_arg, 1);
   set_eba_arg(fs_main_arg, 1, fs);



   // RUN
   eba_op(fs_main_arg);

   // CLEANUP
   opl_destroy(fs_main_opl);
   opl_destroy(fs_init_opl);
   free_eba_arg(fs_main_arg);
   free_eba_arg(fs_init_arg);
}

