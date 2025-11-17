#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "interpreter.h"
#include "reader.h"

int samestr(char *a, char *b)
{
   return !(strcmp(a,b));
}

int confirm_first_word(char **line, char *word)
{
   char *first = line[0];
   if (first == NULL)
   {
      printf("error: empty line passed into confirm_first_word\n");
      return 0;
   }
   if (!samestr(first, word))
   {
      printf("error: expected word %s but instead see word %s\n", word, first);
      return 0;
   }
   else
   {
      return 1;
   }
}

int match_second_word(char **line, char *word)
{
   char *second = line[1];
   if (second == NULL)
   {
      printf("error: one word line passed into confirm_second_word\n");
      return 0;
   }
   return samestr(second, word);
}

int parse_variable(char *word)
{
   if (word == NULL)
   {
      printf("error: NULL word passed to parse_variable\n");
      return -1;
   }
   if (word[0] != 'V')
   {
      printf("error: expected a variable (e.g. V14, V09, V1), got %s\n", word);
      return -1;
   }
   return atoi(word+1);
}

int parse_literal(char *word)
{
   if (word == NULL)
   {
      printf("error: NULL word passed to parse_literal\n");
      return -1;
   }
   if (word[0] != '@')
   {
      printf("error: expected a variable (e.g. @14, @09, @1), got %s\n", word);
      return -1;
   }
   return atoi(word+1);
}


void add_invoke_request(IR_state_t *IRstate, void* target, void* args)
{
   INVOKE_request_t *z = malloc(sizeof(INVOKE_request_t));
   z->code_buf = target;
   z->arg_buf = args;
   z->next = NULL;

   if (IRstate->next_invoke == NULL)
   {
      IRstate->next_invoke = z;
      return;
   }

   INVOKE_request_t *l;
   for(l = IRstate->next_invoke; l->next != NULL; l = l->next)
   {
      ;
   }
   l->next = z;
   return;
}













void run_noop(IR_state_t *IRstate)
{
   IRstate->next_line += 1;
}

void run_bufreq(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "BUFREQ"));

   if (match_second_word(line, "ALLOC_LITERAL"))
   {
      int var = parse_variable(line[2]);
      assert(var >= 0 && var < IR_STATE_SIZE);
      int lit = parse_literal(line[3]);
      assert(lit >= 0);

      void* newbuf = malloc(lit);
      IRstate->vars[var] = (int64_t) newbuf;
   }
   else if (match_second_word(line, "RELEASE"))
   {
      int var = parse_variable(line[2]);
      assert(var >= 0 && var < IR_STATE_SIZE);

      free((void*)(IRstate->vars[var]));
   }
   else
   {
      printf("error: option %s does not exist for BUFREQ\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_literal(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "LITERAL"));

   if (match_second_word(line, "LOAD"))
   {
      int var = parse_variable(line[2]);
      assert(var >= 0 && var < IR_STATE_SIZE);
      int lit = parse_literal(line[3]);
      assert(lit >= 0);

      int64_t* adr = (int64_t*) (IRstate->vars[var]);
      *adr = lit;
   }
   else
   {
      printf("error: option %s does not exist for LITERAL\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_print(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "PRINT"));

   if (match_second_word(line, "BYTES"))
   {
      int var = parse_variable(line[2]);
      assert(var >= 0 && var < IR_STATE_SIZE);
      int lit = parse_literal(line[3]);
      assert(lit >= 0);

      int64_t* adr = (int64_t*) (IRstate->vars[var]);
      int i;
      for(i = 0; i < lit; i++)
      {
         uint8_t byte = ((uint8_t*) adr)[i];
         printf("byte %03d: 0x%02X\n", i, byte);
      }
   }
   else
   {
      printf("error: option %s does not exist for LITERAL\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_line(IR_state_t *IRstate, char **line)
{
   if (line[0] == NULL)
   {
      // it's a no-op
      run_noop(IRstate);
   }

   // now check against all known functions
   else if (samestr(line[0], "BUFREQ"))
   {
      run_bufreq(IRstate, line);
   }
   else if (samestr(line[0], "PRINT"))
   {
      run_print(IRstate, line);
   }
   else if (samestr(line[0], "LITERAL"))
   {
      run_literal(IRstate, line);
   }
   else
   {
      printf("unknown command '%s'\n", line[0]);
   }
}

void run_code(INVOKE_request_t *current_invoke)
{
   // print_code(IRcode);
   char ***IRcode = (*(char****)current_invoke->code_buf);
   IR_state_t *IRstate = init_IR_state();
   IRstate->vars[0] = (int64_t) (current_invoke->arg_buf);


   while (1)
   {
      if (IRcode[IRstate->next_line] == NULL)
      {
         break;
      }

      // printf("now running line %d\n", IRstate->next_line);
      run_line(IRstate, IRcode[IRstate->next_line]);
   }

   while(IRstate->next_invoke != NULL)
   {
      INVOKE_request_t *current_invoke = IRstate->next_invoke;
      // TODO: this recursion will eventually crash the program because of
      //       call depth. Can leave it until the demo but then will need
      //       to do something better
      run_code(current_invoke);

      IRstate->next_invoke = current_invoke->next;


      free(current_invoke);

   }

   free_IR_state(IRstate);
}


IR_state_t * init_IR_state(void)
{
   IR_state_t *IRstate;
   IRstate = malloc(sizeof(IR_state_t));
   IRstate->vars = calloc(IR_STATE_SIZE, sizeof(int64_t));
   IRstate->next_line = 0;


   return IRstate;
}

void free_IR_state(IR_state_t *IRstate)
{
   if (IRstate == NULL)
   {
      printf("warning, IRstate passed as NULL\n");
      return;
   }

   free(IRstate->vars);
   free(IRstate);
}
