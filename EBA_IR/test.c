#include <stdio.h>
#include <stdlib.h>
#include "reader.h"
#include "interpreter.h"


int main(void)
{
   printf("EBA tester\n");

   // char ***IRcode = full_read("examples/EBA_IR_CIRC_INIT.EBA");
   char ***IRcode = full_read("examples/SHORT.EBA");

   INVOKE_request_t *starter_invoke = malloc(sizeof(INVOKE_request_t));
   starter_invoke->code_buf = malloc(sizeof(void*));
   starter_invoke->code_buf = (void*) &IRcode;
   starter_invoke->arg_buf = malloc(sizeof(void*));
   starter_invoke->arg_buf = (void*)NULL;


   run_code(starter_invoke);
   full_free(IRcode);
}
