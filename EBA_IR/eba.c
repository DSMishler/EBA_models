#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "eba.h"

#include "prog1_glob.h"
#include "eba_utils.h"

op_loader_t op_loader_boot;

void eba_op(void *arg)
{
   op_loader_t *opl = *((op_loader_t **) arg);
   (opl->fn)(arg);
}

// no checks verison, but a version with checks can be found in eba_utils.c
// and is very useful for debugging. Set it in `load_op` as needed
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
   // now, it is not guaranteed what kind of structure this was called in.
   // EBA puts this as the initial function pointer for everything.
   // For simplicity, we will not force the user to keep track of whether their
   // op is loaded - we'll just run it after loading quietly
   (*op_ds->fn)(arg);
}


int main(void)
{
   check_eba_assumptions();

   op_loader_t *opl1 = opl_init("./boot.so", "boot");

   void *my_eba_arg = malloc(sizeof(op_loader_t*)+sizeof(void**));
   void **SCAFFOLD_dlcloseme = malloc(sizeof(void*));
   set_eba_arg(my_eba_arg, 0, opl1);
   set_eba_arg(my_eba_arg, 1, SCAFFOLD_dlcloseme);

   eba_op(my_eba_arg); // boot!

   // scaffold code to free the rest of the cleanup code
   dlclose(opl1->handler);
   dlclose(*SCAFFOLD_dlcloseme);
   free(SCAFFOLD_dlcloseme);
   free(opl1);
}
