#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eba.h"
#include "prog1_glob.h"

op_loader_t op_loader_eshell;
op_loader_t op_loader_eir;

void boot(void *eba_arg)
{

   global_data_t *gd = malloc(sizeof(global_data_t));
   gd->nopls = 32;
   gd->opls = malloc(gd->nopls*sizeof(op_loader_t*));
   gd->nfrargs = 128;
   gd->frargs = malloc(gd->nfrargs*sizeof(void*));
   int i;
   for(i = 0; i < gd->nfrargs; i++)
   {
      gd->frargs[i] = NULL;
   }

   gd->frargs[0] = eba_arg;


   uint64_t w_thread = 0;
   op_loader_t *op_loader_demo = malloc(sizeof(op_loader_t));
   op_loader_demo->fname = "./libs/EIRtest.so";
   op_loader_demo->op_name = "run_demo";
   op_loader_demo->fn = load_op;
   gd->opls[0] = op_loader_demo;

   op_loader_t *op_loader_cleanup = malloc(sizeof(op_loader_t));
   op_loader_cleanup->fname = "./libs/cleanup.so";
   op_loader_cleanup->op_name = "cleanup";
   op_loader_cleanup->fn = load_op;
   gd->opls[1] = op_loader_cleanup;



   char *which_op = "circ_buf_demo";
   eba_states[w_thread] = eba_op;
   char *eba_secondword = malloc((strlen(which_op)+1)*sizeof(char));
   strcpy(eba_secondword, which_op);
   void *eir_arg = malloc(sizeof(op_loader_t*)+sizeof(global_data_t*)+sizeof(char*));
   // next eba arg:
      // A - the op loader
      // B - the global data structure
      // C - arg to the op in question (string of which op to exec)
   memcpy(eir_arg, &op_loader_demo, sizeof(op_loader_t*));
   memcpy((char*)eir_arg+sizeof(op_loader_t*), &gd, sizeof(global_data_t*));
   memcpy((char*)eir_arg+sizeof(op_loader_t*)+sizeof(global_data_t*), &eba_secondword, sizeof(char*));
   eba_args[w_thread] = eir_arg;

   return;
}
