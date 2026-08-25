#include "eir_common.h"

void free_later(EIR_data_t *gd, void *free_me)
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
