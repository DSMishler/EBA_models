#include <dlfcn.h>
// taken from https://mprtmma.medium.com/c-shared-library-dynamic-loading-eps-2-28f0a109250a

#include <stdio.h>
#include <string.h>

void printone(void)
{
   printf("one is 1\n");
}

int main(void)
{
   void *object;
   char *error;
   void *handler;

   handler = dlopen("./targetfunc.so", RTLD_LAZY);

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

   object = dlsym(handler, "myfunc");
   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }
   if (object == NULL)
   {
      printf("there is no object!\n");
   }

   int (*thetargetfunc)(void);
   memcpy(&thetargetfunc, &object, sizeof(thetargetfunc));
   // thetargetfunc = object;

   int myretval;
   myretval = thetargetfunc();
   printf("returned %d\n", myretval);

   dlclose(handler);

   return 0;
}
