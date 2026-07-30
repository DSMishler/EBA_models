#include "eba.h"


// main idea
// Keep a queue of continuations
// mark some as ready, not ready, etc.
// possibly accept signals
// When called, take the first continuation,
//    push it to the back of the queue, and run it
// for compatability, require that one of the elements
// of its eba arg is the scheduler to call back when done,
// and we can just put ourselves.
// how should I interject the "next_op" in a nested scheduler
// system?


void *sched_run_wrap(void *arg_thread)
{
   uint64_t w_thread = 0;
   if (arg_thread != NULL)
   {
      w_thread = *((uint64_t*)arg_thread);
      free(arg_thread);
   }
   return sched_run(w_thread)
}

void *sched_run(uint64_t w_thread)
{
   while(1)
   {
      // void*0 is guaranteed to copare unequal to any object or function.
      // see note under EBA_run.
      if (next_continuation == (void*)0)
      {
         break;
      }
      (*next_continuation)(next_arg);
   }
   return NULL;
}

void *sched_init(void *args)
{
   // args is a void* with
   // w_thread
   // queue_size // do I need this? Maybe standardize it?
   // first continuation
   // first arg
   // vvs: voidvoidstar
   void (*sample_vvs)(void*) = (void*)0;

   
   sizeof(sample_vvs)
}
