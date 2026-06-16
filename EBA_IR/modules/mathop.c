#include "interpreter.h"

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
   else if (match_second_word(line, "ADD_D64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      double* dest_adr = (double*) (dest_buf);
      double* op_a_adr = (double*) (op_a_buf);
      double* op_b_adr = (double*) (op_b_buf);
   
      *dest_adr = *op_a_adr + *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "MUL_D64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      double* dest_adr = (double*) (dest_buf);
      double* op_a_adr = (double*) (op_a_buf);
      double* op_b_adr = (double*) (op_b_buf);

      *dest_adr = *op_a_adr * *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "SUB_D64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      double* dest_adr = (double*) (dest_buf);
      double* op_a_adr = (double*) (op_a_buf);
      double* op_b_adr = (double*) (op_b_buf);

      *dest_adr = *op_a_adr - *op_b_adr;

      buf_free_if_shorthand(dest_buf, line[2]);
      buf_free_if_shorthand(op_a_buf, line[3]);
      buf_free_if_shorthand(op_b_buf, line[4]);
   }
   else if (match_second_word(line, "DIV_D64"))
   {
      void *dest_buf = parse_var_buf(line[2], IRstate);
      void *op_a_buf = parse_var_buf(line[3], IRstate);
      void *op_b_buf = parse_var_buf(line[4], IRstate);

      double* dest_adr = (double*) (dest_buf);
      double* op_a_adr = (double*) (op_a_buf);
      double* op_b_adr = (double*) (op_b_buf);

      *dest_adr = *op_a_adr / *op_b_adr;

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

