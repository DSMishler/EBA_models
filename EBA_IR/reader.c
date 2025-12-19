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

   // TODO: it may be wise to not implement labels and jumping, but this
   //       code will do this for now.

   labels_to_lines(IRcode);


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

int samestr(char *a, char *b)
{
   return !(strcmp(a,b));
}

int is_label(char *a)
{
   return ((a[strlen(a)-2]) == ':');
}

// basically itoa but I malloc as well and only do base 10.
static char * get_numstr(int num, int colon)
{
   if (colon != 1 && colon != 0)
   {
      printf("invalid argument passed for boolean 'colon'\n");
      return NULL;
   }
   int needed_space = 1+colon;
   // to fix the corner case, we will (sloppily) shoehorn 0 into 1 for space
   // calculation purposes
   int tnum = num + (num == 0);
   if (num < 0)
   {
      printf("invalid argument passed for 'num'. Must be nonnegative.\n");
      return NULL;
   }
   while(tnum > 0)
   {
      tnum /= 10;
      needed_space += 1;
   }

   char *newword = malloc(needed_space*sizeof(char));

   newword[needed_space-1] = '\0';
   int i;
   int vnum = num;
   for(i = needed_space-2-colon; i >= 0; i--)
   {
      int val = vnum % 10;
      vnum /= 10;
      newword[i] = ((char) val) + '0';
   }

   if (colon)
   {
      newword[needed_space-1-colon] = ':';
   }
   
   // printf("just sent numstr with num=%d colon=%d to %s\n", num, colon, newword);

   return newword;
}

void labels_to_lines(char ***IRcode)
{
   int i;
   for(i = 0; IRcode[i] != NULL; i++)
   {
      char *first_word_i = IRcode[i][0];
      if (first_word_i == NULL)
      {
         ;
      }
      else if (!(is_label(first_word_i)))
      {
         ;
      }
      else
      {
         // the line is not blank and the first word is a label
         // we're about to change it anyway, so let's just go ahead and
         // make them match
         char *label = first_word_i;
         int label_loc = i;
         printf("label_loc = %d\n", label_loc);
         label[strlen(label)-1] = '\0';
         int j;
         for(j = 0; IRcode[j] != NULL; j++)
         {
            char *first_word_j = IRcode[j][0];
            if (first_word_j == NULL)
            {
               ; // ingore empty line
            }
            else if (!(samestr(first_word_j, "CMP")))
            {
               ; // ignore non-labels
            }
            else
            {
               // cmp line.
               // then we've got to check if the label is a match
               if(IRcode[j][1] && IRcode[j][2] && IRcode[j][3] && IRcode[j][4])
               {
                  if (samestr(IRcode[j][4], label))
                  {
                     free(IRcode[j][4]);
                     IRcode[j][4] = get_numstr(label_loc, 0);
                  }
               }
               else
               {
                  printf("warning: incorrect syntax on CMP line %d\n", j);
                  printf("requires 5 arguments\n.");
               }
            }
         }

         free(IRcode[i][0]);
         IRcode[i][0] = get_numstr(label_loc, 1);
      }
   }
}
