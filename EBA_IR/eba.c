#include <dlfcn.h>

#include "interpreter.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
   void *object;
   char *error;
   void *handler;

   handler = dlopen("./libs/EIRtest.so", RTLD_LAZY | RTLD_GLOBAL);

   if (!handler)
   {
      printf("%s\n", dlerror());
      return -1;
   }

   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }

   object = dlsym(handler, "main");
   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }
   if (object == NULL)
   {
      printf("there is no object!\n");
   }

   int (*EIR)(void);
   memcpy(&EIR, &object, sizeof(EIR));

   EIR();

   dlclose(handler);

   return 0;
}
