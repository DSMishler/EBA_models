#include "eba.h"
#include "prog1_glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void cleanup(void* eba_arg)
{
   printf("cleanup called!\n");

   global_data_t *gd = *(global_data_t**)((char*)eba_arg+sizeof(op_loader_t*));

   dlclose(gd->opls[0]->handler);
   free(gd->opls[0]);
   dlclose(gd->opls[2]->handler);
   free(gd->opls[2]);


   int i;
   for(i = 0; i < gd->nfrargs; i++)
   {
      if(gd->frargs != NULL)
      {
         free(gd->frargs[i]);
      }
   }


   free(gd->frargs);

   eba_states[0] = (void*) 0;

   return;

}
