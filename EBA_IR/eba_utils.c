#include "eba_utils.h"
#include <stdlib.h>
#include <stdio.h>


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
