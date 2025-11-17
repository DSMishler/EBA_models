#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int is_ir_wspace(char a)
{
   if (a == ' '  || a == ','  || a == '\t' ||
       a == '\v' || a == '\f' || a == '\r')
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

char *** full_read(char *fname)
{
   FILE *f;
   int flen;
   char *fbuf;
   f = fopen(fname, "r");

   fseek(f, 0, SEEK_END);
   flen = ftell(f);
   fseek(f, 0, SEEK_SET);
   fbuf = malloc(flen*sizeof(char));
   fread(fbuf, sizeof(char), flen, f);

   fclose(f);


   int newlines = 0;
   int i;
   for(i = 0; i < flen; i++)
   {
      // printf("fbuf[%d] = %d\n", i, (int)fbuf[i]);
      if (fbuf[i] == '\n')
      {
         newlines += 1;
      }
   }

   char ***IRcode;
   IRcode = malloc((newlines+1)*sizeof(char**));
   // printf("malloc-ed 0x%0lx\n", (long int)IRcode);
   IRcode[newlines] = NULL;
   int offset = 0;
   int j;
   for (i = 0; i < newlines; i++)
   {
      IRcode[i] = line_to_words(fbuf+offset);
      for(j = 0; j+offset < flen && fbuf[j+offset] != '\n'; j++)
      {
         ; // advances j until the next newline
      }
      offset += (j+1); //advances to right after the next newline
   }

   free(fbuf);
   return IRcode;
}

char ** line_to_words(char *line)
{
   // simple finite state machine
   int state = 0; // 0: in whitespace, 1: in word
   int i; // index through the line
   int zw; // index of which word we're on
   int zi; // index of which character in the word we're on

   int lens[10] = {0,0,0,0,0,0,0,0,0,0};
   // limit of 10 words per line.
   char next;

   // first pass: find out how much needs malloc-ed
   state = zi = i = zw = 0;
   while (1)
   {
      next = line[i];
      // check for comments, newlines, or eof
      if (next == '#' || next == '\n' || next == '\0')
      {
         zw += state; // possibly add 1 to nwords
         break;
      }
      if (next == '/' && line[i+1] == '/')
      {
         // printf("comment detected\n");
         zw += state; // possibly add 1 to nwords
         break;
      }

      if (state == 1)
      {
         if (is_ir_wspace(next))
         {
            state = 0;
            zw++;
            zi = 0;
            if (zw == 10)
            {
               printf("error: too many words on line '%s'\n", line);
               return NULL;
            }
         }
         else
         {
            zi++;
            lens[zw] = zi;
         }
      }
      else if (state == 0)
      {
         if (is_ir_wspace(next))
         {
            ; // pass
         }
         else
         {
            state = 1;
            zi++;
         }
      }

      i++;
   }
   // safe to malloc
   // for(i = 0; i < 10; i++)
   // {
      // printf("lens[%d] = %d\n", i, lens[i]);
   // }

   char **words = malloc((zw+1)*(sizeof(char*)));
   // printf("  malloc-ed 0x%0lx\n", (long int)words);
   for(i = 0; i < zw; i++)
   {
      words[i] = malloc((lens[i]+1)*sizeof(char));
      // printf("    malloc-ed 0x%0lx\n", (long int)words[i]);
      words[i][lens[i]] = '\0';
   }
   words[i] = NULL;

   // now actually write the data
   state = zi = i = zw = 0;
   while (1)
   {
      next = line[i];
      // check for comments, newlines, or eof
      if (next == '#' || next == '\n' || next == '\0')
      {
         break;
      }
      if (next == '/' && line[i+1] == '/')
      {
         break;
      }

      if (state == 1)
      {
         if (is_ir_wspace(next))
         {
            state = 0;
            zw++;
            zi = 0;
         }
         else
         {
            words[zw][zi] = line[i];
            zi++;
         }
      }
      else if (state == 0)
      {
         if (is_ir_wspace(next))
         {
            ; // pass
         }
         else
         {
            state = 1;
            words[zw][zi] = line[i];
            zi++;
         }
      }

      i++;
   }

   return words;
}


void full_free(char ***IRcode)
{
   if (IRcode == NULL)
   {
      printf("warning: attempt to free NULL code\n");
      return;
   }
   int i, j;
   for(i = 0; IRcode[i] != NULL; i++) // free the lines
   {
      for(j = 0; IRcode[i][j] != NULL; j++) // free the words
      {
         // printf("free i=%d, j=%d\n", i, j);
         // printf("    address 0x%0lx\n", (long int)IRcode[i][j]);
         free(IRcode[i][j]);
      }
      // printf("free i=%d\n", i);
      // printf("  address 0x%0lx\n", (long int)IRcode[i]);
      free(IRcode[i]);
   }
   // printf("free entire array\n");
   // printf("address 0x%0lx\n", (long int)IRcode);
   free(IRcode);
}

void print_code(char ***IRcode)
{
   if (IRcode == NULL)
   {
      printf("warning: attempt to print NULL code\n");
      return;
   }
   int i, j;
   for(i = 0; IRcode[i] != NULL; i++)
   {
      printf("line %d:", i);
      for(j = 0; IRcode[i][j] != NULL; j++)
      {
         printf(" %s", IRcode[i][j]);
      }
      printf("\n");
   }
}
