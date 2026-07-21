#include "eba.h"
#include <stdio.h>

void cleanup(void* eba_arg)
{
   printf("cleanup called!\n");

   eba_states[0] = (void*) 0;

   return;

}
