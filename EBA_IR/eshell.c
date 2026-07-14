#include "eshell.h"
#include "eba.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dlfcn.h>

op_loader_t op_loader_eirdemo;

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
   op_loader_t *opl = &op_loader_eirdemo;
   opl->fname = "./libs/EIRtest.so";
   opl->op_name = "run_demo";
   opl->fn = load_op;

   op_loader_t **oplp2;
   oplp2 = (op_loader_t**)     *(void**)((char*)eba_arg+sizeof(uint64_t)+sizeof(op_loader_t*));
   memcpy(oplp2, &opl, sizeof(op_loader_t*));
   // *(void**)((char*) eba_arg + sizeof(uint64_t) + sizeof(op_loader_t*)) = opl;

   void *adj_eba_arg = (char*)eba_arg + sizeof(op_loader_t*);
   // casting to char* to keep compiler happy
   uint64_t w_thread = *((uint64_t*)adj_eba_arg);
   free(eba_arg); //TODO: this is really, really sloppy but for now it will do
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
      // void (*demo)(void*) = (void*)0;
      if (secondword == NULL)
      {
         printf("error: what do I load?\n");
      }
      else if(!(strcmp(secondword, "circ_buf_demo")) || !(strcmp(secondword, "stream_demo")))
      {
         eba_states[w_thread] = eba_op;
         // void *handler;
         // handler = dl_loader_voidvoidstar(&demo, "./libs/EIRtest.so", "run_demo");
         // eba_states[w_thread] = demo;
         char *eba_secondword = malloc((strlen(secondword)+1)*sizeof(char));
         strcpy(eba_secondword, secondword);
         void *eir_arg = malloc(sizeof(op_loader_t*)+sizeof(char*));
         memcpy(eir_arg, &opl, sizeof(op_loader_t*));
         memcpy((char*)eir_arg+sizeof(op_loader_t*), &eba_secondword, sizeof(char*));
         eba_args[w_thread] = eir_arg;
         printf("eshell v opl is 0x%lx\n", (uint64_t)opl);
         printf("handler of opl is 0x%lx\n", (uint64_t)&opl->handler);
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
