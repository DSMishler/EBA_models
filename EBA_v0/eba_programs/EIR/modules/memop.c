#include "interpreter.h"

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

      if (len == 0)
      {
         printf("warning: TRANSFER_WITH_OFFSET with length of zero!\n");
      }

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

