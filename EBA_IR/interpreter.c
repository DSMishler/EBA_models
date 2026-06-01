#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "interpreter.h"
#include "reader.h"

#define MAX_THREADS 16

// global function pointer for the next operation to run (w/MAX_THREADS threads)
void (*eba_states[MAX_THREADS])(void*);
// global arg pointer for EBA's arg (w/MAX_THREADS threads)
void *eba_args[MAX_THREADS];

int confirm_first_word(char **line, char *word)
{
   char *first = line[0];
   if (first == NULL)
   {
      fprintf(stderr, "error: empty line passed into confirm_first_word\n");
      return 0;
   }
   if (!samestr(first, word))
   {
      fprintf(stderr, "error: expected word %s but instead see word %s\n", word, first);
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
      fprintf(stderr, "error: one word line passed into confirm_second_word\n");
      return 0;
   }
   return samestr(second, word);
}

int parse_variable(char *word)
{
   if (word == NULL)
   {
      fprintf(stderr, "error: NULL word passed to parse_variable\n");
      return -1;
   }
   if (word[0] != 'V')
   {
      fprintf(stderr, "error: expected a variable (e.g. V14, V09, V1), got %s\n", word);
      return -1;
   }
   return atoi(word+1);
}

void* parse_var_buf(char *word, IR_state_t *IRstate)
{
   if (word == NULL)
   {
      fprintf(stderr, "error: NULL word passed to parse_variable\n");
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
      fprintf(stderr, "error: expected a variable or '&' buf (e.g. V14, V09, V1, &5), got %s\n", word);
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
      fprintf(stderr, "error: NULL word passed to parse_literal\n");
      exit(1);
   }
   if (word[0] != '@')
   {
      fprintf(stderr, "error: expected a literal (e.g. @14, @09, @1), got %s\n", word);
      exit(1);
   }
   char *strend;
   uint64_t retval = strtoull(word+1, &strend, 10);
   assert(strend != NULL);
   return retval;
}


void var_errmsg(char *func, int line)
{
   fprintf(stderr, "%s on line %d: variable out of range\n", func, line+1);
   exit(1);
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
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("BUFREQ ALLOC", IRstate->next_line);
      }
      void *allocation_len_buf = parse_var_buf(line[3], IRstate);

      int64_t allocation_len = *((int64_t *)allocation_len_buf);

      void* newbuf = malloc(allocation_len);
      // printf("buf alloc-ed: 0x%lx\n", (uint64_t)newbuf);
      IRstate->vars[var_dest] = (int64_t) newbuf;

      buf_free_if_shorthand(allocation_len_buf, line[3]);
   }
   else if (match_second_word(line, "ALLOC_REMOTE"))
   {
      void *remote_buf_id_buf = parse_var_buf(line[2], IRstate);
      int64_t remote_buf_id = *((int64_t *)remote_buf_id_buf);
      remote_buf_id += 0; // keep compilers happy while it's unused

      int var_dest = parse_variable(line[3]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("BUFREQ ALLOC_REMOTE", IRstate->next_line);
      }

      void *allocation_len_buf = parse_var_buf(line[4], IRstate);
      int64_t allocation_len = *((int64_t *)allocation_len_buf);

      void* newbuf = malloc(allocation_len);
      // printf("buf alloc-ed: 0x%lx\n", (uint64_t)newbuf);
      IRstate->vars[var_dest] = (int64_t) newbuf;

      buf_free_if_shorthand(remote_buf_id_buf, line[2]);
      buf_free_if_shorthand(allocation_len_buf, line[4]);
   }
   else if (match_second_word(line, "ALLOC_LITERAL"))
   {
      // alloc_literal should NOT be deprecated because it would be
      // necessary if the shorthand of "&2", "&10003", etc. was not possible.
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("BUFREQ ALLOC_LITERAL", IRstate->next_line);
      }
      uint64_t lit_allocation_len = parse_literal(line[3]);
      assert(lit_allocation_len >= 0);

      void* newbuf = malloc(lit_allocation_len);
      IRstate->vars[var_dest] = (int64_t) newbuf;
   }
   else if (match_second_word(line, "RELEASE"))
   {
      int var_target = parse_variable(line[2]);
      if (var_target < 0 || var_target >= IR_STATE_SIZE)
      {
         var_errmsg("BUFREQ RELEASE", IRstate->next_line);
      }

      // printf("buf release-ed: 0x%lx\n", (uint64_t)(IRstate->vars[var_target]));
      free((void*)(IRstate->vars[var_target]));
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for BUFREQ\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_memop(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "MEMOP"));

   if (match_second_word(line, "LOAD_LITERAL"))
   {
      int var_dest_buf = parse_variable(line[2]);
      if (var_dest_buf < 0 || var_dest_buf >= IR_STATE_SIZE)
      {
         var_errmsg("MEMOP LOAD_LITERAL", IRstate->next_line);
      }
      uint64_t lit_value = parse_literal(line[3]);

      uint64_t* adr = (uint64_t*) (IRstate->vars[var_dest_buf]);
      *adr = lit_value;
   }
   else if (match_second_word(line, "READ_FROMBUF"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("MEMOP READ_FROMBUF", IRstate->next_line);
      }

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
      if (var_src < 0 || var_src >= IR_STATE_SIZE)
      {
         var_errmsg("MEMOP WRITE_TOBUF", IRstate->next_line);
      }

      int64_t value = IRstate->vars[var_src];

      int64_t *dest_addr = (int64_t*) ((int64_t) dest_buf + *((int64_t*)offset_buf));

      *dest_addr = value;

      buf_free_if_shorthand(dest_buf,   line[2]);
      buf_free_if_shorthand(offset_buf, line[3]);
   }
   else if (match_second_word(line, "MOVE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("MEMOP MOVE", IRstate->next_line);
      }
      int var_src = parse_variable(line[3]);
      if (var_src < 0 || var_src >= IR_STATE_SIZE)
      {
         var_errmsg("MEMOP MOVE", IRstate->next_line);
      }

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
      fprintf(stderr, "error: option %s does not exist for MEMOP\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_invoke(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "INVOKE"));

   if (match_second_word(line, "LOCAL"))
   {
      int var_args_buf = parse_variable(line[2]);
      if (var_args_buf < 0 || var_args_buf >= IR_STATE_SIZE)
      {
         var_errmsg("INVOKE LOCAL", IRstate->next_line);
      }

      uint64_t* args_addr = (uint64_t*) (IRstate->vars[var_args_buf]);

      IRstate->code_buf = (char***)args_addr;
      IRstate->next_line = -1; // because 1 will be added at the end
   }
   else if (match_second_word(line, "LOCAL_BUF"))
   {
      int var_args_buf = parse_variable(line[2]);
      if (var_args_buf < 0 || var_args_buf >= IR_STATE_SIZE)
      {
         var_errmsg("INVOKE LOCAL_BUF", IRstate->next_line);
      }

      uint64_t* args_addr = (uint64_t*) (IRstate->vars[var_args_buf]);

      fprintf(stderr, "warning: LOCAL_BUF is deprecated and will soon no longer be supported!\n");

      IRstate->code_buf = (((char****)args_addr)[0]);
      IRstate->next_line = -1; // because 1 will be added at the end
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for INVOKE\n", line[1]);
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
      fprintf(stderr, "error: option %s does not exist for MATHOP\n", line[1]);
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
      fprintf(stderr, "error: option %s does not exist for CMP\n", line[1]);
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
      fprintf(stderr, "error: option %s does not exist for PRINT\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_log(IR_state_t *IRstate, char **line)
{
   assert(confirm_first_word(line, "LOG"));

   if(line[2] == NULL)
   {
      fprintf(stderr, "error: LOG requires third argument to be a filename!\n");
      assert(0); // crasht the program
   }

   if (strlen(line[2]) < 8)
   {
      fprintf(stderr, "warning: LOG requires third argument to end in .eirout\n");
      fprintf(stderr, "your argument '%s' is not long enough to do so meaningfully\n", line[2]);
      // skip the line and move on
      IRstate->next_line += 1;
      return;
   }
   if (!(samestr(line[2] + strlen(line[2])-7, ".eirout")))
   {
      fprintf(stderr, "warning: LOG requires third argument to end in .eirout\n");
      fprintf(stderr, "your argument '%s' does not.\n", line[2]);
      // skip the line and move on
      IRstate->next_line += 1;
      return;
   }

   // no matter what, we'll have to grab the filename
   // do that now, and close it at the end.
   FILE *fp = fopen(line[2], "a");

   if (match_second_word(line, "BYTES"))
   {
      void *target_buf = parse_var_buf(line[3], IRstate);
      void *len_buf    = parse_var_buf(line[4], IRstate);

      int64_t* adr = (int64_t*) target_buf;
      int64_t len = *(int64_t*) len_buf;
      int i;
      for(i = 0; i < len; i++)
      {
         uint8_t byte;
         // uint64_t word = ((uint64_t*) adr)[i / 8];
         // byte = (uint8_t) (word >> ((7 - (i % 8)) * 8) & ((uint8_t) 0xff));
         byte = ((uint8_t*)adr)[i];
         fprintf(fp, "byte %03d: 0x%02X\n", i, byte);
      }

      buf_free_if_shorthand(target_buf, line[3]);
      buf_free_if_shorthand(len_buf,    line[4]);
   }
   else if (match_second_word(line, "WORD"))
   {
      void *target_buf = parse_var_buf(line[3], IRstate);
      uint64_t* adr = (uint64_t*) target_buf;
      fprintf(fp, "0x%lX\n", *adr);
      buf_free_if_shorthand(target_buf, line[3]);
   }
   else if (match_second_word(line, "STRING"))
   {
      assert(line[3] != NULL);
      fprintf(fp, "%s\n", line[3]);
   }
   else if (match_second_word(line, "VAR"))
   {
      void *target = parse_var_buf(line[3], IRstate);
      fprintf(fp, "0x%lX\n", (uint64_t)(target));
      buf_free_if_shorthand(target, line[3]);
   }
   else if (match_second_word(line, "RESET"))
   {
      fclose(fp);
      fp = fopen(line[2], "w"); // to reset the file
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for LOG\n", line[1]);
   }

   fclose(fp);

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
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD CODEREAD", IRstate->next_line);
      }

      assert(line[3] != NULL);
      IRstate->vars[var_dest] = (uint64_t) full_read(line[3]);
   }
   else if (match_second_word(line, "CODEFREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD CODEFREE", IRstate->next_line);
      }
      full_free((char***)IRstate->vars[var_dest]);
   }
   else if (match_second_word(line, "TERMINATE_WITH_CODEFREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD TERMINATE_WITH_CODEFREE", IRstate->next_line);
      }
      full_free((char***)IRstate->vars[var_dest]);
      eba_states[IRstate->w_thread] = &eba_free_IR_state;
   }
   else if (match_second_word(line, "P_SEM_INIT"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_INIT", IRstate->next_line);
      }
      void *psem = malloc(sizeof(sem_t));
      sem_init(psem, 0, 0);
      IRstate->vars[var_dest] = (int64_t) psem;
   }
   else if (match_second_word(line, "P_SEM_WAIT"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_WAIT", IRstate->next_line);
      }
      void *psem = (void*) IRstate->vars[var_dest];
      sem_wait(psem);
   }
   else if (match_second_word(line, "P_SEM_POST"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_POST", IRstate->next_line);
      }
      void *psem = (void*) IRstate->vars[var_dest];
      sem_post(psem);
   }
   else if (match_second_word(line, "P_SEM_FREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_FREE", IRstate->next_line);
      }
      void *psem = (void*) IRstate->vars[var_dest];
      sem_destroy(psem);
      free(psem);
   }
   else if (match_second_word(line, "P_LOCK_INIT"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_INIT", IRstate->next_line);
      }
      void *plock = malloc(sizeof(pthread_mutex_t));
      pthread_mutex_init(plock, NULL);
      IRstate->vars[var_dest] = (int64_t) plock;
   }
   else if (match_second_word(line, "P_LOCK_ACK"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_ACK", IRstate->next_line);
      }
      void *plock = (void*) IRstate->vars[var_dest];
      pthread_mutex_lock(plock);
   }
   else if (match_second_word(line, "P_LOCK_REL"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_REL", IRstate->next_line);
      }
      void *plock = (void*) IRstate->vars[var_dest];
      pthread_mutex_unlock(plock);
   }
   else if (match_second_word(line, "P_LOCK_FREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_FREE", IRstate->next_line);
      }
      void *plock = (void*) IRstate->vars[var_dest];
      pthread_mutex_destroy(plock);
      free(plock);
   }
   else if (match_second_word(line, "PTHREAD_SPAWN_HEAVY"))
   {
      void *arg_buf = parse_var_buf(line[2], IRstate);

      uint64_t *p_w_thread = ((uint64_t **)arg_buf)[1];
      uint64_t w_thread = *p_w_thread;
      if (w_thread >= MAX_THREADS)
      {
         fprintf(stderr, "error: thread %lu not allowed (max is %d). Stop.\n",
                  w_thread, MAX_THREADS-1);
         exit(1);
      }

      void *targ = malloc(sizeof(uint64_t));
      *((uint64_t*)targ) = w_thread; // will be free-ed later
      
      pthread_t tids[1];

      eba_states[w_thread] = &run_code;
      eba_args[w_thread] = arg_buf;

      // printf("creating thread with targ pointing to %lu\n", w_thread);
      pthread_create(tids, NULL, EBA_run, targ);
      pthread_detach(tids[0]);


      buf_free_if_shorthand(arg_buf, line[2]);
   }
   else if (match_second_word(line, "PTHREAD_GET_TID"))
   {
      void *tid_buf = parse_var_buf(line[2], IRstate);
      
      *((uint64_t*)tid_buf) = IRstate->w_thread;

      buf_free_if_shorthand(tid_buf, line[2]);
   }
   else if (match_second_word(line, "PTHREAD_GET_AVAIL"))
   {
      void *avl_buf = parse_var_buf(line[2], IRstate);
      
      //NOTE: only thread-safe is only thread 0 spawns other threads
      int i;
      for (i = 0; i < MAX_THREADS; i++)
      {
         if (eba_states[i] == (void*)0)
         {
            break;
         }
      }
      uint64_t w_thread = i;
      if (w_thread >= MAX_THREADS)
      {
         fprintf(stderr, "error: no thread is available! Refusing spawn.\n");
         IRstate->next_line += 1;
         return;
      }

      *((uint64_t*)avl_buf) = w_thread;

      buf_free_if_shorthand(avl_buf, line[2]);
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for SCAFFOLD\n", line[1]);
   }

   IRstate->next_line += 1;
}

void run_line(void* lcl_eba_arg)
{
   IR_state_t *IRstate = (IR_state_t*)lcl_eba_arg;
   char **line = IRstate->code_buf[IRstate->next_line];
   if (line == NULL)
   {
      // printf("thread %lu detecting termination\n", IRstate->w_thread);
      eba_states[IRstate->w_thread] = &eba_free_IR_state;
      // do no work, and return here
      return;
   }

   // printf("t%lu running line %d:", IRstate->w_thread, IRstate->next_line+1);
   // for(int i = 0; line[i] != NULL; i++)
   // {
      // printf(" %s", line[i]);
   // }
   // printf("\n");

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
   else if (samestr(line[0], "LOG"))
   {
      run_log(IRstate, line);
   }
   else if (samestr(line[0], "SCAFFOLD"))
   {
      run_scaffold(IRstate, line);
   }
   else
   {
      fprintf(stderr, "unknown command '%s' on line %d\n",
              line[0], IRstate->next_line);
      exit(1);
   }
}

void run_code(void* lcl_eba_arg)
{
   char ****arg_buf = (char****)lcl_eba_arg;
   char*** code_buf = ((char****)(lcl_eba_arg))[0];
   uint64_t* p_w_thread = ((uint64_t**)(lcl_eba_arg))[1];
   uint64_t w_thread = *p_w_thread;
   IR_state_t *IRstate = init_IR_state();

   // Note: we do NOT zero out all the other vars.
   // Advocate for zeroing: not doing this may possibly add a vulnerability
   // Advocate against: doing this makes things less minimal
   IRstate->w_thread = w_thread;
   IRstate->vars[0] = (int64_t) (arg_buf);
   IRstate->next_line = 0;
   IRstate->code_buf = code_buf;

   eba_args[IRstate->w_thread] = (void*)(IRstate);
   eba_states[IRstate->w_thread] = &run_line;
}

void* EBA_run(void *arg)
{
   uint64_t w_thread = 0;
   if (arg != NULL)
   {
      w_thread = *((uint64_t*)arg);
      free(arg);
   }
   while(1)
   {
      // NOTE: void*0 (nullptr) is guaranteed to compare unequal
      // to any object or function, so this can only happen
      // via the intential setting of eba_state to 0
      if (eba_states[w_thread] == (void*)0)
      {
         break;
      }
      (*eba_states[w_thread])(eba_args[w_thread]);
   }
   return NULL;
}

IR_state_t * init_IR_state(void)
{
   IR_state_t *IRstate;
   IRstate = malloc(sizeof(IR_state_t));
   IRstate->w_thread = -1;
   // NOTE: this is NOT in the standard to give you calloc-ed memory,
   //       only malloc-ed is guaranteed
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
   eba_args[IRstate->w_thread] = NULL;
   eba_states[IRstate->w_thread] = (void*)0;
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
