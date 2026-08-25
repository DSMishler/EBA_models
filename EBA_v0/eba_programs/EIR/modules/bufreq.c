#include "interpreter.h"

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
   else if (match_second_word(line, "RELEASE_REMOTE"))
   {
      void *remote_buf_id_buf = parse_var_buf(line[2], IRstate);
      int64_t remote_buf_id = *((int64_t *)remote_buf_id_buf);
      remote_buf_id += 0; // keep compilers happy while it's unused

      int var_target = parse_variable(line[3]);
      if (var_target < 0 || var_target >= IR_STATE_SIZE)
      {
         var_errmsg("BUFREQ RELEASE", IRstate->next_line);
      }

      // printf("buf release-ed: 0x%lx\n", (uint64_t)(IRstate->vars[var_target]));
      free((void*)(IRstate->vars[var_target]));
      buf_free_if_shorthand(remote_buf_id_buf, line[2]);
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for BUFREQ\n", line[1]);
   }

   IRstate->next_line += 1;
}

