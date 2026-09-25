#include "eba_common.h"
#include "fifo_sched_common.h"

#include <stdlib.h> // for srand & rand

// creates a FIFO scheduler with no interrupts and unlimited wait queue.
// Uses EBA features to manage its memory during its lifespan

void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("scheduler demo\n");
   // SCHEDULER INIT

   if (sizeof(void*) < sizeof(FILE*))
   {
      printf("ERROR: pointer size assumptions not met. Aborting.\n");
      return;
   }

   op_loader_t *fs_main_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "sched_run");
   op_loader_t *fs_init_opl = opl_init("eba_programs/fifo_sched/fifo_sched.so", "sched_init");

   void *fs_main_arg = init_eba_arg(2);
   void *fs_init_arg = init_eba_arg(2);

   set_eba_arg(fs_main_arg, 0, fs_main_opl);
   set_eba_arg(fs_init_arg, 0, fs_init_opl);

   eba_op(fs_init_arg);

   fifo_sched_t *fs = get_eba_arg(fs_init_arg, 1);
   set_eba_arg(fs_main_arg, 1, fs);

   // add the length checker
   op_loader_t *write_length_opl = opl_init("eba_programs/fifo_sched/write_length.so", "write_length");

   void *wl_arg = init_eba_arg(3);
   set_eba_arg(wl_arg, 0, write_length_opl);
   set_eba_arg(wl_arg, 1, fs);
   FILE *logf = fopen("write_length.log", "w");
   set_eba_arg(wl_arg, 2, logf);

   void *add_wl_to_sched_args = init_eba_arg(3);
   set_eba_arg(add_wl_to_sched_args, 0, fs->add_opl);
   set_eba_arg(add_wl_to_sched_args, 1, fs);
   set_eba_arg(add_wl_to_sched_args, 2, wl_arg);
   eba_op(add_wl_to_sched_args);

   // set up the noiser
   op_loader_t *noiser_opl = opl_init("eba_programs/fifo_sched/sched_noise.so", "sched_noise");

   void *noiser_arg = init_eba_arg(3);
   set_eba_arg(noiser_arg, 0, noiser_opl);
   set_eba_arg(noiser_arg, 1, fs);
   int64_t *noiser_iterp = malloc(sizeof(int64_t));
   *noiser_iterp = 0;
   set_eba_arg(noiser_arg, 2, noiser_iterp);

   void *add_to_sched_args = init_eba_arg(3);
   set_eba_arg(add_to_sched_args, 0, fs->add_opl);
   set_eba_arg(add_to_sched_args, 1, fs);
   set_eba_arg(add_to_sched_args, 2, noiser_arg);
   eba_op(add_to_sched_args);

   // set up random seed
   // (guaranteeing the same result every time on a given seed)
   srand(1);


   // RUN
   eba_op(fs_main_arg);

   // CLEANUP
   opl_destroy(fs_main_opl);
   opl_destroy(fs_init_opl);
   opl_destroy(noiser_opl);
   free_eba_arg(fs_main_arg);
   free_eba_arg(fs_init_arg);
}

