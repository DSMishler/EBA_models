#include "eshell.h"
#include "eba.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dlfcn.h>


void echo_and_block(void *eba_arg)
{
   printf("EBA_shell: ");
   char line[MAX_LINE_LEN+1];

   fgets(line, MAX_LINE_LEN+1, stdin);

   // ensure the line is "legal"
   if (((line[strlen(line)-1]) != EOF) && ((line[strlen(line)-1]) != '\n'))
   {
      printf("warning: the line read beginning with '%s' is not valid. "
             "Perhaps it is longer than %d characters?\n",
             line, MAX_LINE_LEN);
      return;
   }

   printf("%s", line);
}

void blocking_get_cmd(void *eba_arg)
{
   uint64_t w_thread = *((uint64_t*)eba_arg);
   printf("EBA_shell: ");
   char line[MAX_LINE_LEN+1];

   fgets(line, MAX_LINE_LEN+1, stdin);

   // ensure the line is "legal"
   if (((line[strlen(line)-1]) != EOF) && ((line[strlen(line)-1]) != '\n'))
   {
      printf("warning: the line read beginning with '%s' is not valid. "
             "Perhaps it is longer than %d characters?\n",
             line, MAX_LINE_LEN);
      return;
   }

   // for now, let's only consider two-worders
   int i;
   int second_word_exists = 0;
   for(i = 0; line[i] != '\0'; i++)
   {
      if ((line[i] == ' ') || (line[i] == '\n'))
      {
         second_word_exists = 1;
         line[i] = '\0';
         break;
      }
   }
   char *firstword = line;
   char *secondword;
   if (second_word_exists)
   {
      secondword = line+strlen(line)+1;
      for(i = 0; secondword[i] != '\0'; i++)
      {
         if ((secondword[i] == ' ') || (secondword[i] == '\n'))
         {
            secondword[i] = '\0';
            break;
         }
      }
   }
   else
   {
      secondword = NULL;
   }

   if (!(strcmp(firstword, "exit")))
   {
      printf("exiting!\n");
      eba_states[w_thread] = (void*)0;
   }

   else if (!(strcmp(firstword, "load")))
   {
      void (*demo)(void*) = (void*)0;
      if (secondword == NULL)
      {
         printf("error: what do I load?\n");
      }
      else if(!(strcmp(secondword, "circ_buf_demo")) || !(strcmp(secondword, "stream_demo")))
      {
         void *handler;
         handler = dl_loader_voidvoidstar(&demo, "./libs/EIRtest.so", "run_demo");
         eba_states[w_thread] = demo;
         char *eba_secondword = malloc((strlen(secondword)+1)*sizeof(char));
         strcpy(eba_secondword, secondword);
         eba_args[w_thread] = (void*)eba_secondword;
         // dlclose(handler); // TODO: when do we close this?
      }
      else
      {
         printf("error: I don't have that module '%s' to load\n", secondword);
      }
   }
   else
   {
      printf("unknown command '%s'", firstword);
   }

}
