#include "interpreter.h"

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

      // fprintf(stderr, "warning: LOCAL_BUF is deprecated and will soon no longer be supported!\n");

      IRstate->code_buf = (((char****)args_addr)[0]);
      IRstate->next_line = -1; // because 1 will be added at the end
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for INVOKE\n", line[1]);
   }
   IRstate->next_line += 1;
}
