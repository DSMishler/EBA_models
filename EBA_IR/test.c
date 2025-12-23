#include <stdio.h>
#include <stdlib.h>
#include "reader.h"
#include "interpreter.h"


void test_solofile(char *);
void test_circ_init(void);

int main(void)
{
   printf("EBA tester\n");
   // test_solofile("examples/CMP.EBA");
   test_circ_init();
}

void test_solofile(char *fname)
{
   char ***IRcode;
   INVOKE_request_t *starter_invoke;
   IRcode = full_read(fname);

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = malloc(sizeof(void*));
   ((char****)starter_invoke->arg_buf)[0] = IRcode;
   starter_invoke->next = NULL;

   void *arg_buf = starter_invoke->arg_buf;

   run_code(starter_invoke);
      
   free(arg_buf);
   full_free(IRcode);
}

void test_circ_init(void)
{
   char ***IRcode;
   void *arg_buf;
   INVOKE_request_t *starter_invoke;
   IRcode = full_read("examples/EBA_IR_CIRC_INIT.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = malloc(3*sizeof(void*));
   ((void**)starter_invoke->arg_buf)[0] = (void*)IRcode;
   ((void**)starter_invoke->arg_buf)[1] = malloc(5*sizeof(void*));
   // can hold the five  buffers
   int64_t bufsz = 400;
   ((int64_t**)starter_invoke->arg_buf)[2] = &(bufsz);
   starter_invoke->next = NULL;
   arg_buf = starter_invoke->arg_buf;
   // grab the circular buffer for later
   void* circ_buf = ((void**)starter_invoke->arg_buf)[1];

   run_code(starter_invoke);


   free(arg_buf);
   full_free(IRcode);
   
   // now write a lil bit into the circular buffer
   IRcode = full_read("examples/EBA_IR_CIRC_WRITE.EBA");
   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = malloc(4*sizeof(void*));
   ((void**)starter_invoke->arg_buf)[0] = (void*)IRcode;
   ((void**)starter_invoke->arg_buf)[1] = circ_buf;
   char *msg = "abcdefghijklmnopqrstuvwxyz";
   int64_t len = 15;
   ((void**)starter_invoke->arg_buf)[2] = (void*) msg;
   ((void**)starter_invoke->arg_buf)[3] = (void*) &len;
   starter_invoke->next = NULL;
   arg_buf = starter_invoke->arg_buf;

   run_code(starter_invoke);

   free(arg_buf);
   full_free(IRcode);

   // now free the cirular buffer
   IRcode = full_read("examples/EBA_IR_CIRC_FREE.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = malloc(2*sizeof(void*));
   ((void**)starter_invoke->arg_buf)[0] = (void*)IRcode;
   ((void**)starter_invoke->arg_buf)[1] = circ_buf;
   starter_invoke->next = NULL;
   arg_buf = starter_invoke->arg_buf;

   run_code(starter_invoke);

   free(arg_buf);
   full_free(IRcode);

}
