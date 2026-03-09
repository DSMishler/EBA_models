#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "interpreter.h"
#include "reader.h"

// global function pointer for the next operation to run
void (*eba_state)(void*);
// global arg pointer for EBA's arg
void *eba_arg = NULL;

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

void* parse_var_buf(char *word, IR_state_t *IRstate)
{
   if (word == NULL)
   {
      printf("error: NULL word passed to parse_variable\n");
      return NULL;
   }
   void *retval;
   if (word[0] == 'V')
   {
      // then it's a variable
      int which_var = parse_variable(word);
      assert(which_var >= 0 && which_var < IR_STATE_SIZE);
      retval = (void*)IRstate->vars[which_var];
   }
   else if (word[0] == '&')
   {
      char *strend;
      uint64_t myval = strtoull(word+1, &strend, 10);
      assert(strend != NULL);
      retval = malloc(sizeof(uint64_t));
      *(uint64_t*)retval = myval;
   }
   else
   {
      printf("error: expected a variable or '&' buf (e.g. V14, V09, V1, &5), got %s\n", word);
      return NULL;
   }
   return retval;
}

void buf_free_if_shorthand(void *buf, char *word)
{
   if (word[0] == '&')
   {
      // then we alloc-ed the buf earlier and should free it
      free(buf);
   }
   else
   {
      ; // if it was a variable, no need to do any freeing
   }
}

// TODO: change all of this to int64s
uint64_t parse_literal(char *word)
{
   if (word == NULL)
   {
      printf("error: NULL word passed to parse_literal\n");
      return -1;
   }
   if (word[0] != '@')
   {
      printf("error: expected a literal (e.g. @14, @09, @1), got %s\n", word);
      return -1;
   }
   char *strend;
   uint64_t retval = strtoull(word+1, &strend, 10);
   assert(strend != NULL);
   return retval;
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
      void *allocation_len_buf = parse_var_buf(line[3], IRstate);

      int64_t allocation_len = *((int64_t *)allocation_len_buf);

      void* newbuf = malloc(allocation_len);
      // printf("buf alloc-ed: 0x%lx\n", (uint64_t)newbuf);
      IRstate->vars[var_dest] = (int64_t) newbuf;

      buf_free_if_shorthand(allocation_len_buf, line[3]);
   }
   else if (match_second_word(line, "ALLOC_LITERAL"))
   {
      // alloc_literal should NOT be deprecated because it would be
      // necessary if the shorthand of "&2", "&10003", etc. was not possible.
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);
      uint64_t lit_allocation_len = parse_literal(line[3]);
      assert(lit_allocation_len >= 0);

      void* newbuf = malloc(lit_allocation_len);
      IRstate->vars[var_dest] = (int64_t) newbuf;
   }
   else if (match_second_word(line, "RELEASE"))
   {
      int var_target = parse_variable(line[2]);
      assert(var_target >= 0 && var_target < IR_STATE_SIZE);

      // printf("buf release-ed: 0x%lx\n", (uint64_t)(IRstate->vars[var_target]));
      free((void*)(IRstate->vars[var_target]));
   }
   else
   {
      printf("error: option %s does not exist for BUFREQ\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_memop(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "MEMOP"));

   if (match_second_word(line, "LOAD_LITERAL"))
   {
      int var_dest_buf = parse_variable(line[2]);
      assert(var_dest_buf >= 0 && var_dest_buf < IR_STATE_SIZE);
      uint64_t lit_value = parse_literal(line[3]);

      uint64_t* adr = (uint64_t*) (IRstate->vars[var_dest_buf]);
      *adr = lit_value;
   }
   else if (match_second_word(line, "READ_FROMBUF"))
   {
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);

      void *src_buf    = parse_var_buf(line[3], IRstate);
      void *offset_buf = parse_var_buf(line[4], IRstate);

      // adding the offest of the literal before cast
      // to avoid having to wrestle with pointer artithmetic later

      int64_t* src_addr = (int64_t*) ((int64_t) src_buf + *((int64_t*)offset_buf));

      IRstate->vars[var_dest] = *src_addr;

      buf_free_if_shorthand(src_buf,    line[3]);
      buf_free_if_shorthand(offset_buf, line[4]);
      
   }
   else if (match_second_word(line, "WRITE_TOBUF"))
   {
      void *dest_buf   = parse_var_buf(line[2], IRstate);
      void *offset_buf = parse_var_buf(line[3], IRstate);

      int var_src = parse_variable(line[4]);
      assert(var_src >= 0 && var_src < IR_STATE_SIZE);

      int64_t value = IRstate->vars[var_src];

      int64_t *dest_addr = (int64_t*) ((int64_t) dest_buf + *((int64_t*)offset_buf));

      *dest_addr = value;

      buf_free_if_shorthand(dest_buf,   line[2]);
      buf_free_if_shorthand(offset_buf, line[3]);
   }
   else if (match_second_word(line, "MOVE"))
   {
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);
      int var_src = parse_variable(line[3]);
      assert(var_src >= 0 && var_src < IR_STATE_SIZE);

      IRstate->vars[var_dest] = IRstate->vars[var_src];
   }
   else if (match_second_word(line, "TRANSFER_WITH_OFFSET"))
   {
      void *dest_buf        = parse_var_buf(line[2], IRstate);
      void *dest_offset_buf = parse_var_buf(line[3], IRstate);
      void *src_buf         = parse_var_buf(line[4], IRstate);
      void *src_offset_buf  = parse_var_buf(line[5], IRstate);
      void *len_buf         = parse_var_buf(line[6], IRstate);

      int64_t dest_addr   = (int64_t) dest_buf;
      int64_t dest_offset = *((int64_t*) dest_offset_buf);
      int64_t src_addr    = (int64_t) src_buf;
      int64_t src_offset  = *((int64_t*) src_offset_buf);
      int64_t len  = *((int64_t*) len_buf);

      // TODO: double check how this is written and if we really want
      //       these pointers stored just as normal int64_t's
      memcpy((void*)(dest_addr+dest_offset), (void*)(src_addr+src_offset), len);

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(dest_offset_buf, line[3]);
      buf_free_if_shorthand(src_buf, line[4]);
      buf_free_if_shorthand(src_offset_buf, line[5]);
      buf_free_if_shorthand(len_buf, line[6]);

   }
   else
   {
      printf("error: option %s does not exist for MEMOP\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_transfer(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "TRANSFER"));

   if (match_second_word(line, "OFFSET_LITERAL"))
   {
      // deprecated
      printf("WARNING: TRANSFER OFFSET_LITERAL will soon be deprecated\n");
      int var_dest_buf = parse_variable(line[2]);
      assert(var_dest_buf >= 0 && var_dest_buf < IR_STATE_SIZE);
      uint64_t lit_dest_offset = parse_literal(line[3]);
      assert(lit_dest_offset >= 0);
      int var_src_buf = parse_variable(line[4]);
      assert(var_src_buf >= 0 && var_src_buf < IR_STATE_SIZE);
      uint64_t lit_src_offset = parse_literal(line[5]);
      assert(lit_src_offset >= 0);
      uint64_t lit_len = parse_literal(line[6]);
      assert(lit_len >= 0);

      uint64_t dest_offset = (uint64_t) lit_dest_offset;
      uint64_t src_offset = (uint64_t) lit_src_offset;
      uint64_t len = (uint64_t) lit_len;
      // TODO: double check how this is written and if we really want
      //       these pointers stored just as normal int64_t's
      uint64_t dest_addr = ((uint64_t) (IRstate->vars[var_dest_buf]));
      uint64_t src_addr = ((uint64_t) (IRstate->vars[var_src_buf]));

      memcpy((void*) (dest_addr+dest_offset), (void*) (src_addr+src_offset), len);
   }
   else
   {
      printf("error: option %s does not exist for TRANSFER\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_invoke(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "INVOKE"));

   if (match_second_word(line, "LOCAL_BUF"))
   {
      int var_args_buf = parse_variable(line[2]);
      assert(var_args_buf >= 0 && var_args_buf < IR_STATE_SIZE);

      uint64_t* args_addr = (uint64_t*) (IRstate->vars[var_args_buf]);

      IRstate->code_buf = (((char****)args_addr)[0]);
      IRstate->next_line = -1; // because 1 will be added at the end
   }
   else
   {
      printf("error: option %s does not exist for INVOKE\n", line[1]);
   }
   IRstate->next_line += 1;
}

void run_mathop(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "MATHOP"));
   if (match_second_word(line, "ADD_U64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      uint64_t* dest_adr = (uint64_t*) (dest_buf);
      uint64_t* op_a_adr = (uint64_t*) (op_a_buf);
      uint64_t* op_b_adr = (uint64_t*) (op_b_buf);

      *dest_adr = *op_a_adr + *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "MUL_U64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      uint64_t* dest_adr = (uint64_t*) (dest_buf);
      uint64_t* op_a_adr = (uint64_t*) (op_a_buf);
      uint64_t* op_b_adr = (uint64_t*) (op_b_buf);

      *dest_adr = *op_a_adr * *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "SUB_U64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      uint64_t* dest_adr = (uint64_t*) (dest_buf);
      uint64_t* op_a_adr = (uint64_t*) (op_a_buf);
      uint64_t* op_b_adr = (uint64_t*) (op_b_buf);

      *dest_adr = *op_a_adr - *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "DIV_U64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      uint64_t* dest_adr = (uint64_t*) (dest_buf);
      uint64_t* op_a_adr = (uint64_t*) (op_a_buf);
      uint64_t* op_b_adr = (uint64_t*) (op_b_buf);

      *dest_adr = *op_a_adr / *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "MOD_U64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      uint64_t* dest_adr = (uint64_t*) (dest_buf);
      uint64_t* op_a_adr = (uint64_t*) (op_a_buf);
      uint64_t* op_b_adr = (uint64_t*) (op_b_buf);

      *dest_adr = *op_a_adr % *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else
   {
      printf("error: option %s does not exist for MATHOP\n", line[1]);
   }
   IRstate->next_line += 1;
}


void run_cmp(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "CMP"));
   if (match_second_word(line, "LT"))
   {
      void * op_a_buf = parse_var_buf(line[2], IRstate);
      void * op_b_buf = parse_var_buf(line[3], IRstate);
      uint64_t lit_target = parse_literal(line[4]);

      uint64_t* op_a_addr = (uint64_t*) op_a_buf;
      uint64_t* op_b_addr = (uint64_t*) op_b_buf;

      if (*op_a_addr < *op_b_addr)
      {
         IRstate->next_line = lit_target;
         // NOTE: at the end of this function, the PC will move one,
         // so the effect of this op can be said to jump to the NEXT line.
      }

      buf_free_if_shorthand(op_a_buf, line[2]);
      buf_free_if_shorthand(op_b_buf, line[3]);
   }
   else if (match_second_word(line, "LEQ"))
   {
      void * op_a_buf = parse_var_buf(line[2], IRstate);
      void * op_b_buf = parse_var_buf(line[3], IRstate);
      uint64_t lit_target = parse_literal(line[4]);

      uint64_t* op_a_addr = (uint64_t*) op_a_buf;
      uint64_t* op_b_addr = (uint64_t*) op_b_buf;

      if (*op_a_addr <= *op_b_addr)
      {
         IRstate->next_line = lit_target;
         // NOTE: at the end of this function, the PC will move one,
         // so the effect of this op can be said to jump to the NEXT line.
      }

      buf_free_if_shorthand(op_a_buf, line[2]);
      buf_free_if_shorthand(op_b_buf, line[3]);
   }
   else if (match_second_word(line, "GT"))
   {
      void * op_a_buf = parse_var_buf(line[2], IRstate);
      void * op_b_buf = parse_var_buf(line[3], IRstate);
      uint64_t lit_target = parse_literal(line[4]);

      uint64_t* op_a_addr = (uint64_t*) op_a_buf;
      uint64_t* op_b_addr = (uint64_t*) op_b_buf;

      if (*op_a_addr > *op_b_addr)
      {
         IRstate->next_line = lit_target;
         // NOTE: at the end of this function, the PC will move one,
         // so the effect of this op can be said to jump to the NEXT line.
      }

      buf_free_if_shorthand(op_a_buf, line[2]);
      buf_free_if_shorthand(op_b_buf, line[3]);
   }
   else if (match_second_word(line, "GEQ"))
   {
      void * op_a_buf = parse_var_buf(line[2], IRstate);
      void * op_b_buf = parse_var_buf(line[3], IRstate);
      uint64_t lit_target = parse_literal(line[4]);

      uint64_t* op_a_addr = (uint64_t*) op_a_buf;
      uint64_t* op_b_addr = (uint64_t*) op_b_buf;

      if (*op_a_addr >= *op_b_addr)
      {
         IRstate->next_line = lit_target;
         // NOTE: at the end of this function, the PC will move one,
         // so the effect of this op can be said to jump to the NEXT line.
      }

      buf_free_if_shorthand(op_a_buf, line[2]);
      buf_free_if_shorthand(op_b_buf, line[3]);
   }
   else if (match_second_word(line, "EQ"))
   {
      void * op_a_buf = parse_var_buf(line[2], IRstate);
      void * op_b_buf = parse_var_buf(line[3], IRstate);
      uint64_t lit_target = parse_literal(line[4]);

      uint64_t* op_a_addr = (uint64_t*) op_a_buf;
      uint64_t* op_b_addr = (uint64_t*) op_b_buf;

      if (*op_a_addr == *op_b_addr)
      {
         IRstate->next_line = lit_target;
         // NOTE: at the end of this function, the PC will move one,
         // so the effect of this op can be said to jump to the NEXT line.
      }

      buf_free_if_shorthand(op_a_buf, line[2]);
      buf_free_if_shorthand(op_b_buf, line[3]);
   }
   else if (match_second_word(line, "NEQ"))
   {
      void * op_a_buf = parse_var_buf(line[2], IRstate);
      void * op_b_buf = parse_var_buf(line[3], IRstate);
      uint64_t lit_target = parse_literal(line[4]);

      uint64_t* op_a_addr = (uint64_t*) op_a_buf;
      uint64_t* op_b_addr = (uint64_t*) op_b_buf;

      if (*op_a_addr != *op_b_addr)
      {
         IRstate->next_line = lit_target;
         // NOTE: at the end of this function, the PC will move one,
         // so the effect of this op can be said to jump to the NEXT line.
      }

      buf_free_if_shorthand(op_a_buf, line[2]);
      buf_free_if_shorthand(op_b_buf, line[3]);
   }
   else
   {
      printf("error: option %s does not exist for CMP\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_print(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "PRINT"));

   if (match_second_word(line, "BYTES"))
   {
      void *target_buf = parse_var_buf(line[2], IRstate);
      void *len_buf    = parse_var_buf(line[3], IRstate);

      int64_t* adr = (int64_t*) target_buf;
      int64_t len = *(int64_t*) len_buf;
      int i;
      for(i = 0; i < len; i++)
      {
         uint8_t byte;
         // uint64_t word = ((uint64_t*) adr)[i / 8];
         // byte = (uint8_t) (word >> ((7 - (i % 8)) * 8) & ((uint8_t) 0xff));
         byte = ((uint8_t*)adr)[i];
         printf("byte %03d: 0x%02X\n", i, byte);
      }

      buf_free_if_shorthand(target_buf, line[2]);
      buf_free_if_shorthand(len_buf,    line[3]);
   }
   else if (match_second_word(line, "WORD"))
   {
      void *target_buf = parse_var_buf(line[2], IRstate);
      uint64_t* adr = (uint64_t*) target_buf;
      printf("0x%lX\n", *adr);
      buf_free_if_shorthand(target_buf, line[2]);
   }
   else if (match_second_word(line, "STRING"))
   {
      assert(line[2] != NULL);
      printf("%s\n", line[2]);
   }
   else if (match_second_word(line, "VAR"))
   {
      void *target = parse_var_buf(line[2], IRstate);
      printf("0x%lX\n", (uint64_t)(target));
      buf_free_if_shorthand(target, line[2]);
   }
   else
   {
      printf("error: option %s does not exist for PRINT\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_scaffold(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "SCAFFOLD"));

   if (match_second_word(line, "SYSTEM"))
   {
      assert(line[2] != NULL);
      system(line[2]);
   }
   else if (match_second_word(line, "CODEREAD"))
   {
      int var_dest = parse_variable(line[2]);
      assert(var_dest >= 0 && var_dest < IR_STATE_SIZE);
      assert(line[3] != NULL);

      IRstate->vars[var_dest] = (uint64_t) full_read(line[3]);
   }
   else if (match_second_word(line, "CODEFREE"))
   {
      int var_dest = parse_variable(line[2]);

      full_free((char***)IRstate->vars[var_dest]);
   }
   else
   {
      printf("error: option %s does not exist for SCAFFOLD\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_line(void* lcl_eba_arg)
{
   IR_state_t *IRstate = (IR_state_t*)lcl_eba_arg;
   char **line = IRstate->code_buf[IRstate->next_line];
   if (line == NULL)
   {
      eba_state = &eba_free_IR_state;
      // do no work, and return here
      return;
   }

   if (line[0] == NULL)
   {
      // it's a no-op
      run_noop(IRstate);
   }
   else if (is_label(line[0]))
   {
      run_noop(IRstate);
      // printf("flag: detected a label (no-op) on line %d\n", IRstate->next_line);
   }

   // now check against all known functions
   else if (samestr(line[0], "BUFREQ"))
   {
      run_bufreq(IRstate, line);
   }
   else if (samestr(line[0], "MEMOP"))
   {
      run_memop(IRstate, line);
   }
   else if (samestr(line[0], "TRANSFER"))
   {
      run_transfer(IRstate, line);
   }
   else if (samestr(line[0], "INVOKE"))
   {
      run_invoke(IRstate, line);
   }
   else if (samestr(line[0], "MATHOP"))
   {
      run_mathop(IRstate, line);
   }
   else if (samestr(line[0], "CMP"))
   {
      run_cmp(IRstate, line);
   }
   else if (samestr(line[0], "PRINT"))
   {
      run_print(IRstate, line);
   }
   else if (samestr(line[0], "SCAFFOLD"))
   {
      run_scaffold(IRstate, line);
   }
   else
   {
      printf("unknown command '%s' on line %d\n", line[0], IRstate->next_line);
      exit(1);
   }
}

void run_code(void* lcl_eba_arg)
{
   void *arg_buf = (void*)(lcl_eba_arg);
   IR_state_t *IRstate = init_IR_state();

   // Possible TODO: zero out all the other vars.
   // Advocate for: not doing this may possibly add a vulnerability
   // Advocate against: doing this makes things less minimal
   IRstate->vars[0] = (int64_t) (arg_buf);
   // char ***IRcode = (((char****)current_invoke->arg_buf)[0]);
   IRstate->next_line = 0;
   IRstate->code_buf = (((char****)arg_buf)[0]);
   // print_code(IRcode);

   eba_arg = (void*)(IRstate);
   eba_state = &run_line;
}

void EBA_run(void)
{
   while(1)
   {
      // TODO: change this. This is a SCAFFOLD at the moment
      // I could see very easily many cases where
      // you'd WANT eba_arg to be null
      if (eba_arg == NULL)
      {
         break;
      }
      (*eba_state)(eba_arg);
   }
}


IR_state_t * init_IR_state(void)
{
   IR_state_t *IRstate;
   IRstate = malloc(sizeof(IR_state_t));
   IRstate->vars = calloc(IR_STATE_SIZE, sizeof(int64_t));
   IRstate->next_line = 0;
   IRstate->code_buf = NULL;


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

void eba_free_IR_state(void* lcl_eba_arg)
{
   IR_state_t *IRstate = (IR_state_t *)lcl_eba_arg;
   eba_arg = NULL;
   free_IR_state(IRstate);
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
