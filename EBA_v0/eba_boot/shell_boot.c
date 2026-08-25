#include "eba_common.h"

#include <dirent.h>

static int ends_in_dotso(char *a)
{
   int l = strlen(a);
   return !(strcmp(a+l-3, ".so"));
}

void boot(void *eba_arg)
{
   // here, eba_arg is just the op loader. We don't anticipate needing to use it
   // now we will show the visible contents of the eba_programs directory
   // and allow the user to choose one to boot

   printf("eba boot selection:\n");

   char *path = "eba_programs";
   DIR *dir = opendir(path);
   if (!dir)
   {
      perror(path);
      return;
   }

   struct dirent *entry;
   for(entry = readdir(dir); entry != NULL; entry = readdir(dir))
   {
      if (ends_in_dotso(entry->d_name))
      {
         printf("%s ", entry->d_name);
      }
   }
   printf("\n");

   printf("please select one of the above to run\n");



   closedir(dir);
   return;
}
