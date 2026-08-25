#include "eba_common.h"


void prog_entry(void* args)
{
   free_eba_arg(args);
   printf("hello!\n");
}
