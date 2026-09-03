#include "eba_common.h"

#include <dirent.h>

#define MAX_LINE_LEN 80

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
   free_eba_arg(eba_arg);

   printf("eba boot selection:\n");

   char *path = "eba_programs";
   DIR *dir = opendir(path);
   if (!dir)
   {
      perror(path);
      return;
   }


   // blocking load. This boot code allows to user to select
   // different demos without the requirement of needing to change the code
   // before the demo is loaded.
   struct dirent *entry;
   char *which_prog = NULL;
   char line[MAX_LINE_LEN+1];
   while (1)
   {
      seekdir(dir, 0);
      for(entry = readdir(dir); entry != NULL; entry = readdir(dir))
      {
         if (ends_in_dotso(entry->d_name))
         {
            printf("%s ", entry->d_name);
         }
      }
      printf("\n");

      printf("please select one of the above to run: ");


      fgets(line, MAX_LINE_LEN+1, stdin);
      // ensure the line is "legal"
      if (((line[strlen(line)-1]) != EOF) && ((line[strlen(line)-1]) != '\n'))
      {
         printf("warning: the line read beginning with '%s' is not valid. "
                "Perhaps it is longer than %d characters?\n\n",
                line, MAX_LINE_LEN);
         while(getchar() != '\n')
         {
            ;
         }
         continue;
      }
      // trim the newline
      line[strlen(line)-1] = '\0';

      // see if it mathches
      seekdir(dir, 0);
      for(entry = readdir(dir); entry != NULL; entry = readdir(dir))
      {
         if (ends_in_dotso(entry->d_name) && !strncmp(line, entry->d_name, strlen(line)))
         {
            if (strlen(entry->d_name) > MAX_LINE_LEN)
            {
               printf("error: somehow a program with name of length over %d was compiled\n", MAX_LINE_LEN);
               printf("       program name: \"%s\"\n", entry->d_name);

            }
            strcpy(line, entry->d_name); // autocomplete the line
            which_prog = line;
            break;
         }
      }
      if (which_prog != NULL)
      {
         break;
      }
      // if we get here, the entered file was no good
      // maybe they said exit?
      if (!strcmp(line, "exit"))
      {
         break;
      }
      printf("Error, no '.so' file matching \"%s\"\n", line);
   }
   closedir(dir);

   if (which_prog == NULL)
   {
      return;
   }

   char *prog_fname = malloc(strlen("eba_programs/")+strlen(line)+1);
   strcpy(prog_fname, "eba_programs/");
   strcat(prog_fname, line);
   op_loader_t *op_loader_prog = opl_init(prog_fname, "prog_entry");
   void *prog_arg = init_eba_arg(1);
   set_eba_arg(prog_arg, 0, op_loader_prog);
   eba_op(prog_arg);

   free(prog_fname);
   dlclose(op_loader_prog->handler);
   free(op_loader_prog);

   return;
}
