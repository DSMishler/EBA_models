#include <stdio.h>
#include "../eba_utils.h" // will need to be included through the CMakeLists
#include "../eba.h"


void boot(void* v)
{
   free_eba_arg(v);
   printf("hello EBA!\n");
}
