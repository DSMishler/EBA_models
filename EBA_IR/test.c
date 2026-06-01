#include <stdio.h>
#include <stdlib.h>
#include "reader.h"
#include "interpreter.h"

extern void (*eba_states[16])(void*);
extern void *eba_args[16];

void test_solofile(char *);

int main(void)
{
   // printf("EBA tester\n");
   test_solofile("examples/streaming_test/STARTER.EIR");
   return 0;
}

void test_solofile(char *fname)
{
   char ***IRcode;
   IRcode = full_read(fname);

   uint64_t *p_w_thread = malloc(sizeof(uint64_t));
   *p_w_thread = 0;

   void **arg_buf = malloc(2*sizeof(void*));
   arg_buf[0] = (void*)IRcode;
   arg_buf[1] = (void*)p_w_thread;

   eba_states[0] = &run_code;
   eba_args[0] = (void*)arg_buf;
   EBA_run(NULL);
}
