#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "eba.h"

// global function pointer for the next operation to run (w/MAX_THREADS threads)
void (*eba_states[MAX_THREADS])(void*);
// global arg pointer for EBA's arg (w/MAX_THREADS threads)
void *eba_args[MAX_THREADS];


void* EBA_run(void *arg_thread)
{
   uint64_t w_thread = 0;
   if (arg_thread != NULL)
   {
      w_thread = *((uint64_t*)arg_thread);
      free(arg_thread);
   }
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

void *dl_loader_intvoid(int (**func)(void), char *function_file, char *raw_name)
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


int main(void)
{
   void (*eshell)(void*) = (void*)0;
   void *handler;

   handler = dl_loader_voidvoidstar(&eshell, "./libs/eshell.so", "blocking_get_cmd");
   
   eba_states[0] = eshell;
   uint64_t *my_thread = malloc(sizeof(uint64_t));
   *my_thread = 0;
   uint64_t *my_thread_ebaarg = malloc(sizeof(uint64_t));
   *my_thread_ebaarg = 0;
   eba_args[0] = my_thread_ebaarg;
   EBA_run(my_thread);

   dlclose(handler);

   return 0;
}
