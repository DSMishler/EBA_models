#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eba.h"

// global function pointer for the next operation to run (w/MAX_THREADS threads)
void (*eba_states[MAX_THREADS])(void*);
// global arg pointer for EBA's arg (w/MAX_THREADS threads)
void *eba_args[MAX_THREADS];


op_loader_t op_loader_boot;

void load_op(void *arg)
{
   op_loader_t *op_ds = *((op_loader_t **)arg);
   printf("loading op %s\n", op_ds->op_name);
   // to try to keep some sanity here, we will set it to a
   // void* because (for now) the loader spits out non void*s
   // to avoid redundant loads. This may be changed in what
   // is likely an imminent redesign
   op_ds->fn = (void*)0;
   op_ds->handler = dl_loader_voidvoidstar(&(op_ds->fn), op_ds->fname, op_ds->op_name);
   // // printf("handler is 0x%lx\n", (uint64_t)op_ds->handler);
   // then call the just-loaded op
   (*op_ds->fn)(arg);
}


void* EBA_run(uint64_t w_thread)
{
   while(1)
   {
      // NOTE: void*0 (nullptr) is guaranteed to compare unequal
      // to any object or function, so this can only happen
      // via the intential setting of eba_state to 0
      if (eba_states[w_thread] == (void*)0)
      {
         break;
      }
      (*eba_states[w_thread])(eba_args[w_thread]);
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

void *dl_loader_voidvoidstar(void (**func)(void*), char *function_file, char *raw_name)
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

void eba_op(void *arg)
{
   op_loader_t *opl = *((op_loader_t **) arg);
   (opl->fn)(arg);
}

int main(void)
{
   op_loader_t *opl1 = &op_loader_boot;
   opl1->fname =  "./boot.so";
   opl1->op_name = "boot";
   opl1->fn = load_op;

   void *my_eba_arg = malloc(sizeof(op_loader_t*));
   memcpy(my_eba_arg, &opl1, sizeof(op_loader_t*));

   eba_states[0] = eba_op;
   eba_args[0] = my_eba_arg;
   EBA_run(0);

   free(my_eba_arg);

   dlclose(opl1->handler);
}
