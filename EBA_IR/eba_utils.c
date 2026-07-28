#include "eba_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>


void free_later(global_data_t *gd, void *free_me)
{
   int i;
   for(i = 0; i < gd->nfrargs; i++)
   {
      if (gd->frargs[i] == NULL)
      {
         gd->frargs[i] = free_me;
         break;
      }
   }
   if(i == gd->nfrargs)
   {
      printf("too many args stacked up to free later! stop!\n");
      exit(1);
   }
}

void *dl_loader_voidvoidstar_withchecks(void (**func)(void*), char *function_file, char *raw_name)
{
   if (*func != (void*)0)
   {
      // function is already loaded. Throw a warning and stop!
      printf("loader called to load %s, but it's already loaded\n", raw_name);
      return NULL;
   }
   void *object;
   char *error;
   void *handler;

   handler = dlopen(function_file, RTLD_LAZY | RTLD_GLOBAL);

   if (!handler)
   {
      printf("%s\n", dlerror());
      return NULL;
   }

   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }

   object = dlsym(handler, raw_name);
   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }
   if (object == NULL)
   {
      printf("there is no object!\n");
   }

   memcpy(func, &object, sizeof(*func));

   return handler;
}

op_loader_t * opl_init(char *fname, char *op_name)
{
   op_loader_t *opl = malloc(sizeof(op_loader_t));
   opl->fname = fname;
   opl->op_name = op_name;
   opl->fn = load_op;
   opl->handler = NULL;
   return opl;
}

void check_eba_assumptions(void)
{
   // necessary error check: EBA ASSUMES these are the same size:
   assert(sizeof(void*) == sizeof(op_loader_t*));
   assert(sizeof(void*) == sizeof(global_data_t*));
   assert(sizeof(void*) == sizeof(void**));
   assert(sizeof(void*) == sizeof(char*));
   assert(sizeof(void*) == sizeof(uint64_t*));
}

void *init_eba_arg(int nargs)
{
   return malloc(nargs*sizeof(void*));
}

void *get_eba_arg(void *eba_args, int which_arg)
{
   return ((void**)eba_args)[which_arg];
}

void set_eba_arg(void *eba_args, int which_arg, void *value)
{
   ((void**)eba_args)[which_arg] = value;
}
