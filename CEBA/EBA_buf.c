#include "EBA_buf.h"

// global buffer head (used now for single node EBA)
buf_t *CEBA_BUF_head;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

void print_buf(buf_t *b, char contents)
{
   printf("buffer at 0x%lx allocation 0x%lx of size %u with expiry %u\n",
      (uint64_t) b, (uint64_t)b->allocation, b->size, b->exp);
   if (contents == 'c')
   {
      int i;
      for(i = 0; i < b->size; i++)
      {
         printf("%c", ((char*)b->allocation)[i]);
      }
      printf("\n");
   }
   else if (contents == 'b')
   {
      int i;
      for(i = 0; i < b->size; i++)
      {
         int j;
         unsigned char c = ((unsigned char*)b->allocation)[i];
         for(j = 7; j >= 0; j--)
         {
            int which_bit = 1 << j;
            int bit = (c & which_bit) >> j;
            printf("%d", bit);
         }
         if (i % 8 == 7)
         {
            printf("\n");
         }
         else
         {
            printf(" ");
         }
      }
      printf("\n");
   }
}

static void print_buf_list(buf_t *head, char contents)
{
   buf_t *b;
   for(b = head; b != NULL; b = b->next)
   {
      print_buf(b, contents);
   }
}


static buf_t * alloc_buf(int size, int exp)
{
   buf_t *newbuf = malloc(sizeof(buf_t));

   newbuf->prev = NULL;
   newbuf->next = NULL;
   newbuf->size = size;
   newbuf->exp = exp;
   newbuf->allocation = calloc(size, 1);
   // this calloc is a courtesy - EBA need not guarantee you get
   // data initialized to zero

   return newbuf;
}

static void free_buf(buf_t *b)
{
   free(b->allocation);
   free(b);
}

static buf_t * add_buf_to_list(buf_t *head, buf_t *addme)
{
   if (head == NULL)
   {
      return addme; // then this is a new list, and we added the first element
   }

   buf_t *b, *prev, *next;
   prev = NULL;
   for(b = head; b != NULL; b = b->next)
   {
      if (b->exp > addme->exp)
      {
         break;
      }
      prev = b;
   }
   // b now points to the entry that will be *after* addme
   // now if b went through the whole list, then it should be NULL
   // and prev will be correct still.

   // now the next buffer is certainly b by our search
   next = b;


   addme->next = next;
   addme->prev = prev;

   if (prev != NULL) prev->next = addme;
   if (next != NULL) next->prev = addme;

   if (head->prev == NULL)
   {
      return head;
   }
   else
   {
      assert(addme == head->prev);
      assert(addme->prev == NULL);
      return addme;
   }
}

static buf_t * remove_buf_from_list(buf_t *head, buf_t *rmvme)
{
   buf_t *next, *prev;
   prev = rmvme->prev;
   next = rmvme->next;

   if (prev != NULL) prev->next = next;
   if (next != NULL) next->prev = prev;

   if (rmvme == head)
      return head->next;
   else
      return head;
}

// TODO: that which calls this needs to set CEBA_BUF_head to NULL
// that seems like a nonobvious requirement and not great practice.
// Try to change this.
static void dealloc_list(buf_t *head)
{
   buf_t *b;
   for(b = head; b != NULL; b = head)
   {
      head = remove_buf_from_list(head, b);
      free_buf(b);
   }
}


void CEBA_PRINT_BUFS(char contents)
{
   print_buf_list(CEBA_BUF_head, contents);
}


buf_t * CEBA_BUFREQ(int size, int exp)
{
   buf_t *b = alloc_buf(size, exp);
   CEBA_BUF_head = add_buf_to_list(CEBA_BUF_head, b);
   return b;
}

void CEBA_BUF_DEALLOC(buf_t *b)
{
   CEBA_BUF_head = remove_buf_from_list(CEBA_BUF_head, b);
   free_buf(b);
   return;
}

void CEBA_BUF_FREE_ALL(void)
{
   dealloc_list(CEBA_BUF_head);
   CEBA_BUF_head = NULL;
   return;
}
