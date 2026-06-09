void printone(void);
#include <stdio.h>

int myfunc(void)
{
   printf("this is a dynamically loaded function!\n");
   printone();
   return 5;
}
