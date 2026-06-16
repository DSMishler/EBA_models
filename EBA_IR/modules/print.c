#include "interpreter.h"

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
   else if (match_second_word(line, "STREAM"))
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
         printf("%c", byte);
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

