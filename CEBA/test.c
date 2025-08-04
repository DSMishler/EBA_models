#include <stdio.h>
#include "EBA_buf.h"
#include "EBA_invoke.h"
#include "circular_buffer.h"

#include <stdint.h>
#include <string.h>

void test_basics(void);
void test_circ(void);

int main(void)
{
   test_circ();
   return 0;
}

void test_circ(void)
{
   buf_t *c = circular_buffer_init(50);
   buf_t *head = add_buf_to_list(NULL, c);

   int i;
   for (i = 0; i < 5; i++)
   {
      buf_t *target = ((buf_t **)c->allocation)[i];
      printf("i=%d, adr_target=0x%lx ", i, (uint64_t)target);
      head = add_buf_to_list(head, target);
   }
   printf("\n");


   buf_t *sb = alloc_buf(sizeof(uint32_t), 10);
   head = add_buf_to_list(head, sb);
   buf_t *wb = alloc_buf(70, 10);
   head = add_buf_to_list(head, wb);

   uint32_t size = 50;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(wb->allocation, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 50);
   circular_buffer_write(c, wb, sb);

   size = 40;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(wb->allocation, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", 40);
   circular_buffer_write(c, wb, sb);

   size = 30;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(wb->allocation, "cccccccccccccccccccccccccccccc", 30);
   circular_buffer_write(c, wb, sb);


   // circular_buffer_write(c, wb, sb);


   print_buf_list(head, 'c');
   print_buf_list(head, 'b');


   dealloc_list(head);
   return;
}


void test_basics(void)
{
   char *msg = "hello world my name is Danny";
   printf("hello world!\n");
   buf_t *b = alloc_buf(10, 10);
   buf_t *c = alloc_buf(20, 8);
   buf_t *d = alloc_buf(5, 5);
   buf_t *head = add_buf_to_list(NULL, b);
   head = add_buf_to_list(head, c);
   head = add_buf_to_list(head, d);

   
   CEBA_INVOKE_WRITE(b, 0, 10, msg);
   CEBA_INVOKE_WRITE(c, 0, 20, msg);
   CEBA_INVOKE_WRITE(c, 10, 10, msg);
   CEBA_INVOKE_WRITE(d, 0, 5, msg);


   print_buf_list(head, 'c');
   print_buf_list(head, 'b');

   int32_t ia = (int32_t) 'a';
   int32_t ib = 1;

   CEBA_INVOKE_WRITE(b, 0, 4, &ia);
   CEBA_INVOKE_WRITE(c, 0, 4, &ib);

   CEBA_INVOKE_ADD_I32(d, b, c);

   int32_t ans = *((int32_t*)d->allocation);

   printf("%d\n", ans);
   printf("%c\n", (char)ans);
   print_buf_list(head, 'b');


   dealloc_list(head);

   return;
}
