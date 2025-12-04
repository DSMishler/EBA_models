#include <stdio.h>
#include <stdlib.h>
#include "reader.h"
#include "interpreter.h"


void test_short(void);
void test_circ_init(void);

int main(void)
{
   printf("EBA tester\n");
   test_circ_init();
}

void test_short(void)
{
   char ***IRcode;
   INVOKE_request_t *starter_invoke;
   IRcode = full_read("examples/SHORT.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->code_buf = malloc(sizeof(void*));
   ((char****)starter_invoke->code_buf)[0] = IRcode;
   starter_invoke->arg_buf = malloc(sizeof(void*));
   ((char**)starter_invoke->arg_buf)[0] = NULL;
   starter_invoke->next = NULL;

   run_code(starter_invoke);
      
   free(starter_invoke->code_buf);
   free(starter_invoke->arg_buf);
   free(starter_invoke);
   full_free(IRcode);
}

void test_circ_init(void)
{
   char ***IRcode;
   INVOKE_request_t *starter_invoke;
   IRcode = full_read("examples/EBA_IR_CIRC_INIT.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->code_buf = malloc(sizeof(void*));
   ((char****)starter_invoke->code_buf)[0] = IRcode;
   starter_invoke->arg_buf = malloc(3*sizeof(void*));
   ((void**)starter_invoke->arg_buf)[0] = NULL;
   ((void**)starter_invoke->arg_buf)[1] = malloc(5*sizeof(void*));
   // can hold the five  buffers
   ((int64_t**)starter_invoke->arg_buf)[2] = malloc(sizeof(int64_t));
   *(((int64_t**)starter_invoke->arg_buf)[2]) = 400; // size = 40
   starter_invoke->next = NULL;

   run_code(starter_invoke);

   free(((char**)starter_invoke->arg_buf)[2]);

   // grab the circular buffer for later
   void* circ_buf = ((void**)starter_invoke->arg_buf)[1];

   free(starter_invoke->code_buf);
   free(starter_invoke->arg_buf);
   free(starter_invoke);
   full_free(IRcode);

   // now free the cirular buffer
   IRcode = full_read("examples/EBA_IR_CIRC_FREE.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->code_buf = malloc(sizeof(void*));
   ((char****)starter_invoke->code_buf)[0] = IRcode;
   starter_invoke->arg_buf = malloc(2*sizeof(void*));
   ((void**)starter_invoke->arg_buf)[0] = NULL;
   ((void**)starter_invoke->arg_buf)[1] = circ_buf;
   starter_invoke->next = NULL;

   run_code(starter_invoke);

   free(starter_invoke->code_buf);
   free(starter_invoke->arg_buf);
   free(starter_invoke);
   full_free(IRcode);

}
