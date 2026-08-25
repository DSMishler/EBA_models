#include "interpreter.h"

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
