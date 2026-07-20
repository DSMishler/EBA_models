#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eba.h"

op_loader_t op_loader_eshell;
op_loader_t op_loader_eir;

void boot(void*eba_arg)
{
   op_loader_t *opl1 = &op_loader_eshell;
   opl1->fname =  "./libs/eshell.so";
   opl1->op_name = "blocking_get_cmd";
   opl1->fn = load_op;
   op_loader_t *opl2 = NULL;
   op_loader_t **oplp2 = &opl2;

   op_loader_t *opl3 = &op_loader_eir;
   opl3->fname =  "./libs/EIRtest.so";
   opl3->op_name = "run_code";
   opl3->fn = load_op;

   // printf("0x%lx to 0x%lx\n", (uint64_t)opl, (uint64_t)&opl->handler);

   eba_states[0] = eba_op;
   void *my_eba_arg = malloc(sizeof(op_loader_t*)+sizeof(uint64_t)+sizeof(op_loader_t**));
   memcpy(my_eba_arg, &opl1, sizeof(op_loader_t*));
   *((uint64_t*)((char*)my_eba_arg+sizeof(op_loader_t*))) = 0;
   memcpy((char*)my_eba_arg+sizeof(op_loader_t*)+sizeof(uint64_t), &oplp2, sizeof(op_loader_t**));
   eba_args[0] = my_eba_arg;
   // printf("it's all set up!\n");
   EBA_run(0);

   free(my_eba_arg);
   dlclose(opl1->handler);
   if (opl2 != NULL) dlclose(opl2->handler);
   // dlclose(opl3->handler);
   free(opl2);

   return;
}
