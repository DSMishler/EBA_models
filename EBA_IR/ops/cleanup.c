#include "eba.h"
#include "eba_utils.h"
#include "prog1_glob.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void cleanup(void* eba_arg)
{
   global_data_t *gd = get_eba_arg(eba_arg, 1);

   dlclose(gd->opls[0]->handler); // run_demo
   free(gd->opls[0]);
   // NOTE: CAN'T dlclose our handler (opls1) because that's us!
   *(gd->dlclose_after) = gd->opls[1]->handler;
   free(gd->opls[1]);
   dlclose(gd->opls[2]->handler); // eir run_code
   free(gd->opls[2]);
   dlclose(gd->opls[3]->handler); // eir run_line
   free(gd->opls[3]);
   dlclose(gd->opls[4]->handler); // eir free_eir_state
   free(gd->opls[4]);
   dlclose(gd->opls[5]->handler); // cleanup_demo
   free(gd->opls[5]);
   free(gd->opls);


   int i;
   for(i = 0; i < gd->nfrargs; i++)
   {
      if(gd->frargs != NULL)
      {
         free(gd->frargs[i]);
      }
   }


   free(gd->frargs);
   free(gd);


   free(eba_args[0]);
   eba_args[0] = NULL;

   return;

}
