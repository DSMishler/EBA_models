#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eba.h"
#include "prog1_glob.h"
#include "eba_utils.h"

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

   free_later(gd, eba_arg);
   gd->my_thread = 0;
   gd->stored_arg = NULL;
   gd->dlclose_after = (void**)get_eba_arg(eba_arg, 1);


   op_loader_t *op_loader_demo = opl_init("./libs/EIRtest.so", "run_demo");
   op_loader_t *op_loader_cleanup = opl_init("./libs/cleanup.so", "cleanup");
   
   gd->opls[0] = op_loader_demo;
   gd->opls[1] = op_loader_cleanup;



   // char *which_op = "circ_buf_demo";
   char *which_op = "stream_demo";
   char *eba_secondword = malloc((strlen(which_op)+1)*sizeof(char));
   strcpy(eba_secondword, which_op);
   void *eir_arg = init_eba_arg(3);
   // next eba arg:
      // A - the op loader
      // B - the global data structure
      // C - arg to the op in question (string of which op to exec)
   set_eba_arg(eir_arg, 0, op_loader_demo);
   set_eba_arg(eir_arg, 1, gd);
   set_eba_arg(eir_arg, 2, eba_secondword);
   uint64_t w_thread = 0;
   eba_args[w_thread] = eir_arg;

   return;
}
