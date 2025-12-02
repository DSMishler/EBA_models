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

   if (match_second_word(line, "ALLOC"))
   {
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);
      int var_allocation_len_buf = parse_variable(line[3]);
      assert(var_allocation_len_buf >= 0 && var_allocation_len_buf < IR_STATE_SIZE);

      int64_t allocation_len = *((int64_t *)IRstate->vars[var_allocation_len_buf]);

      void* newbuf = malloc(allocation_len);
      IRstate->vars[var_dest] = (int64_t) newbuf;
   }
   else if (match_second_word(line, "ALLOC_LITERAL"))
   {
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);
      int lit_allocation_len = parse_literal(line[3]);
      assert(lit_allocation_len >= 0);

      void* newbuf = malloc(lit_allocation_len);
      IRstate->vars[var_dest] = (int64_t) newbuf;
   }
   else if (match_second_word(line, "RELEASE"))
   {
      int var_target = parse_variable(line[2]);
      assert(var_target >= 0 && var_target < IR_STATE_SIZE);

      free((void*)(IRstate->vars[var_target]));
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
      int var_dest_buf = parse_variable(line[2]);
      assert(var_dest_buf >= 0 && var_dest_buf < IR_STATE_SIZE);
      int lit_value = parse_literal(line[3]);
      assert(lit_value >= 0);

      int64_t* adr = (int64_t*) (IRstate->vars[var_dest_buf]);
      *adr = lit_value;
   }
   else if (match_second_word(line, "READ"))
   {
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);
      int var_src_buf = parse_variable(line[3]);
      assert(var_src_buf >= 0 && var_src_buf < IR_STATE_SIZE);
      int lit_offset = parse_literal(line[4]);
      assert(lit_offset >= 0);

      // adding the offest of the literal before cast
      // to avoid having to wrestle with pointer artithmetic later
      int64_t* src_addr = (int64_t*) ((IRstate->vars[var_src_buf]) + lit_offset);

      IRstate->vars[var_dest] = *src_addr;
      
   }
   else if (match_second_word(line, "WRITE"))
   {
      int var_dest_buf = parse_variable(line[2]);
      assert(var_dest_buf >= 0 && var_dest_buf < IR_STATE_SIZE);
      int lit_offset = parse_literal(line[3]);
      assert(lit_offset >= 0);
      int var_src = parse_variable(line[4]);
      assert(var_src >= 0 && var_src < IR_STATE_SIZE);

      int64_t value = IRstate->vars[var_src];

      int64_t *dest_addr = (int64_t*) ((IRstate->vars[var_dest_buf]) + lit_offset);

      *dest_addr = value;
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
      int lit_len = parse_literal(line[3]);
      assert(lit_len >= 0);

      int64_t* adr = (int64_t*) (IRstate->vars[var]);
      int i;
      for(i = 0; i < lit_len; i++)
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
      // print_IR_state(IRstate);
   }

   while(IRstate->next_invoke != NULL)
   {
      INVOKE_request_t *current_invoke = IRstate->next_invoke;
      // TODO: this recursion will eventually crash the program because of
      //       call depth. Can leave it until the demo but then will need
      //       to do something better
      // TODO ALSO: the code_buf may be redundant because it should be in arg buf.
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
   IRstate->next_invoke = NULL;


   return IRstate;
}

void print_IR_state(IR_state_t *IRstate)
{
   printf("printing IR state:\n");
   int i;
   for(i = 0; i < IR_STATE_SIZE; i++)
   {
      printf("V%02d: %lx\n", i, IRstate->vars[i]);
   }
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
