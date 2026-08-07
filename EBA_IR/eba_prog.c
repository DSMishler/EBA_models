#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eba.h"
#include "prog1_glob.h"
#include "eba_utils.h"
#define MAX_LINE_LEN 80

op_loader_t op_loader_eshell;
op_loader_t op_loader_eir;

// global arg pointer for EBA's arg (w/MAX_THREADS threads)
void *eba_args[MAX_THREADS];


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


   // blocking load. Yes, it's a little unpolished for now, but this enables
   // different demos without the requirement of needing to change the code
   // before the demo is loaded.
   char *which_op;
   while (1)
   {
      printf("currently installed demos: [circ_buf_demo, stream_demo]\n");
      printf("which demo should run?\n");
      printf("demo: ");
      char line[MAX_LINE_LEN+1];

      fgets(line, MAX_LINE_LEN+1, stdin);

      // ensure the line is "legal"
      if (((line[strlen(line)-1]) != EOF) && ((line[strlen(line)-1]) != '\n'))
      {
         printf("warning: the line read beginning with '%s' is not valid. "
                "Perhaps it is longer than %d characters?\n\n",
                line, MAX_LINE_LEN);
         while(getchar() != '\n')
         {
            ;
         }
         continue;
      }
      // trim the newline
      line[strlen(line)-1] = '\0';

      if (!(strcmp(line, "circ_buf_demo")))
      {
         which_op = "circ_buf_demo";
         break;
      }
      else if (!(strcmp(line, "stream_demo")))
      {
         which_op = "stream_demo";
         break;
      }
      else if (!(strcmp(line, "exit")))
      {
         exit(0);
         break;
      }
      else
      {
         printf("I didn't understand '%s'. Must be one of the listed demos.\n", line);
      }
   }


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

   EBA_run(0);

   return;
}

void* EBA_run(uint64_t w_thread)
{
   while(1)
   {
      if (eba_args[w_thread] == NULL)
      {
         break;
      }
      eba_op(eba_args[w_thread]); // this can go into boot.so
   }
   return NULL;
}

void* EBA_run_wrap(void *arg_thread)
{
   uint64_t w_thread = 0;
   if (arg_thread != NULL)
   {
      w_thread = *((uint64_t*)arg_thread);
      free(arg_thread);
   }
   return EBA_run(w_thread);
}

