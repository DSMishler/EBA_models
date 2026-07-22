#include "eba.h"
#include "prog1_glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void dlclose_wrap(void *h)
{
   // just dlclose, but only if it's not a NULLptr
   if (h != NULL)
   {
      dlclose(h);
   }
}

void cleanup(void* eba_arg)
{
   printf("cleanup called!\n");

   global_data_t *gd = *(global_data_t**)((char*)eba_arg+sizeof(op_loader_t*));

   dlclose_wrap(gd->opls[0]->handler);
   free(gd->opls[0]);
   dlclose_wrap(gd->opls[2]->handler);
   free(gd->opls[2]);
   dlclose_wrap(gd->opls[3]->handler);
   free(gd->opls[3]);
   dlclose_wrap(gd->opls[4]->handler);
   free(gd->opls[4]);
   dlclose_wrap(gd->opls[5]->handler);
   free(gd->opls[5]);


   int i;
   for(i = 0; i < gd->nfrargs; i++)
   {
      if(gd->frargs[i] != NULL)
      {
         // printf("%d: 0x%lx\n", i, (uint64_t)gd->frargs[i]);
         free(gd->frargs[i]);
      }
   }


   free(gd->frargs);

   eba_states[0] = (void*) 0;

   return;

}
