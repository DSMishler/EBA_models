#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "eba.h"

#include "prog1_glob.h"
#include "eba_utils.h"

// global arg pointer for EBA's arg (w/MAX_THREADS threads)
void *eba_args[MAX_THREADS];

op_loader_t op_loader_boot;

void eba_op(void *arg)
{
   op_loader_t *opl = *((op_loader_t **) arg);
   (opl->fn)(arg);
}


void* EBA_run(uint64_t w_thread)
{
   while(1)
   {
      if (eba_args[w_thread] == NULL)
      {
         break;
      }
      eba_op(eba_args[w_thread]);
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

void *dl_loader_voidvoidstar_nochecks(void (**func)(void*), char *function_file, char *raw_name)
{
   void *object;
   void *handler;

   handler = dlopen(function_file, RTLD_LAZY | RTLD_GLOBAL);
   object = dlsym(handler, raw_name);
   memcpy(func, &object, sizeof(*func));

   return handler;
}

void load_op(void *arg)
{
   op_loader_t *op_ds = *((op_loader_t **)arg);
   // printf("loading op %s\n", op_ds->op_name);
   // to try to keep some sanity here, we will set it to a
   // void* because (for now) the loader spits out non void*s
   // to avoid redundant loads. This may be changed in what
   // is likely an imminent redesign
   op_ds->fn = (void*)0;
   op_ds->handler = dl_loader_voidvoidstar_withchecks(&(op_ds->fn), op_ds->fname, op_ds->op_name);
   // our work is done. control will pass back to eba_op,
   // and since eba_arg hasn't changed, it'll just call the same op again!
}


int main(void)
{
   check_eba_assumptions();

   op_loader_t *opl1 = opl_init("./boot.so", "boot");

   void *my_eba_arg = malloc(sizeof(op_loader_t*)+sizeof(void**));
   void **SCAFFOLD_dlcloseme = malloc(sizeof(void*));
   set_eba_arg(my_eba_arg, 0, opl1);
   set_eba_arg(my_eba_arg, 1, SCAFFOLD_dlcloseme);

   eba_args[0] = my_eba_arg;
   EBA_run(0);

   dlclose(opl1->handler);

   // scaffold code to free the rest of the cleanup code
   dlclose(*SCAFFOLD_dlcloseme);
   free(SCAFFOLD_dlcloseme);
   free(opl1);
}
