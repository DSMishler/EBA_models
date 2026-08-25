#include "eba_common.h"


void boot(void* v)
{
   free_eba_arg(v);
   printf("hello EBA!\n");
}
