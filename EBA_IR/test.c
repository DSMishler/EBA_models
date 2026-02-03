#include <stdio.h>
#include <stdlib.h>
#include "reader.h"
#include "interpreter.h"

#include <string.h> // TODO: probably can remove once continuations are added


void test_solofile(char *);
void test_dualfile_invoke_test(void);
void test_circ_invoke(void);
void test_circ_init(void);

int main(void)
{
   printf("EBA tester\n");
   test_solofile("examples/SHORT.EBA");
   // test_dualfile_invoke_test();
   // test_circ_invoke();
   // test_circ_init();
}

void test_solofile(char *fname)
{
   char ***IRcode;
   IRcode = full_read(fname);

   void *arg_buf = malloc(sizeof(void*));
   ((char****)arg_buf)[0] = IRcode;

   run_code(arg_buf);
      
   free(arg_buf);
   full_free(IRcode);
}
/*
void test_dualfile_invoke_test(void)
{
   char ***IRcode1, ***IRcode2;
   INVOKE_request_t *starter_invoke;
   IRcode1 = full_read("examples/TEST_EBA_INVOKE_1.EBA");
   IRcode2 = full_read("examples/TEST_INVOKE.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = malloc(2*sizeof(void*));
   ((char****)starter_invoke->arg_buf)[0] = IRcode1;
   ((char****)starter_invoke->arg_buf)[1] = IRcode2;
   starter_invoke->next = NULL;

   void *arg_buf = starter_invoke->arg_buf;

   run_code(starter_invoke);
      
   free(arg_buf);
   full_free(IRcode1);
   full_free(IRcode2);
}

void test_circ_invoke(void)
{
   char ****IRcodes;
   INVOKE_request_t *starter_invoke;

   IRcodes = malloc(7*sizeof(void*));
   IRcodes[0] = full_read("examples/SEQ_CIRC_INVOKER.EBA");
   IRcodes[1] = full_read("examples/EBA_IR_CIRC_INIT.EBA");
   IRcodes[2] = full_read("examples/EBA_IR_CIRC_FREE.EBA");
   IRcodes[3] = full_read("examples/EBA_IR_CIRC_WRITE.EBA");
   IRcodes[4] = full_read("examples/EBA_IR_CIRC_READ.EBA");
   IRcodes[5] = full_read("examples/CIRC_BUF_PRINT.EBA");
   IRcodes[6] = full_read("examples/CLEANUP.EBA");

   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = (void*) (IRcodes);
   starter_invoke->next = NULL;
   
   printf("running EBA circ buf invoke tester\n");

   run_code(starter_invoke);

   int i;
   for(i = 0; i < 7; i++)
   {
      full_free(IRcodes[i]);
   }
   free(IRcodes);
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

   // read some of what was written back in.
   IRcode = full_read("examples/EBA_IR_CIRC_READ.EBA");
   starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->arg_buf = malloc(4*sizeof(void*));
   ((void**)starter_invoke->arg_buf)[0] = (void*)IRcode;
   ((void**)starter_invoke->arg_buf)[1] = circ_buf;
   char *dbuf = malloc(10*sizeof(char));
   strcpy(dbuf, "zzzzzzzzz");
   int64_t len_read = 5;
   ((void**)starter_invoke->arg_buf)[2] = (void*) dbuf;
   ((void**)starter_invoke->arg_buf)[3] = (void*) &len_read;
   starter_invoke->next = NULL;
   arg_buf = starter_invoke->arg_buf;

   run_code(starter_invoke);

   free(arg_buf);
   full_free(IRcode);

   printf("buf says %s\n", dbuf);
   free(dbuf);

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
*/
