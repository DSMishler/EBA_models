#include "eba_common.h"
#include "fifo_sched_common.h"


// takes a known FIFO scheduler and writes out its length to a file
// if the length of items in the FIFO scheduler is more than 0,
// then this EBA operation reschedules itself
void write_length(void *args)
{
   // args are:
   // 0: this operation
   // 1: the scheduler
   // 2: the file pointer of the file to write to
   fifo_sched_t *fs = get_eba_arg(args, 1);
   FILE *logf = get_eba_arg(args, 2);


   int sched_length;
   // MISSING: calculate the length of the FIFO scheduler's queue. Your code
   // goes here: and no, sched_length is not always 10.
   sched_length = 10;


   // now write the length to file
   fprintf(logf, "%d\n", sched_length);




   // put myself back in the queue if there is more work to be done
   if (fs->arg_block_next == fs->arg_block_end && fs->next_idx == fs->end_idx)
   {
      // then we are the only thing left on the scheduler and we will be done
      free_eba_arg(args);
      fclose(logf);
   }
   else
   {
      // add myself back
      void *add_to_sched_arg = init_eba_arg(3);
      set_eba_arg(add_to_sched_arg, 0, fs->add_opl);
      set_eba_arg(add_to_sched_arg, 1, fs);
      set_eba_arg(add_to_sched_arg, 2, args);
      eba_op(add_to_sched_arg);
   }
}
