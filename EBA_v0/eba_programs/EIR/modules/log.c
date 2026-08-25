#include "interpreter.h"

static int samestr(char *a, char *b)
{
   return !(strcmp(a,b));
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

