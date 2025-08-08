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

   circular_buffer_print(c);
   printf("--------------------------------------------\n");

   buf_t *sb = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *wb = CEBA_BUFREQ(70, 10);
   buf_t *rb = CEBA_BUFREQ(70, 10);


   uint32_t size = 50;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(wb->allocation, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 50);
   circular_buffer_write(c, wb, sb);
   circular_buffer_print(c);
   printf("--------------------------------------------\n");

   size = 45;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(rb->allocation, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 50);
   circular_buffer_read(c, rb, sb);
   circular_buffer_print(c);
   print_buf(rb, 'c');
   printf("--------------------------------------------\n");

   size = 40;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(wb->allocation, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", 40);
   circular_buffer_write(c, wb, sb);
   circular_buffer_print(c);
   printf("--------------------------------------------\n");

   size = 30;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(rb->allocation, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 50);
   circular_buffer_read(c, rb, sb);
   circular_buffer_print(c);
   print_buf(rb, 'c');
   printf("--------------------------------------------\n");

   size = 35;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(wb->allocation, "cccccccccccccccccccccccccccccccccccccccc", 35);
   circular_buffer_write(c, wb, sb);
   circular_buffer_print(c);
   printf("--------------------------------------------\n");

   size = 50;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(rb->allocation, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 50);
   circular_buffer_read(c, rb, sb);
   circular_buffer_print(c);
   print_buf(rb, 'c');
   printf("--------------------------------------------\n");

   size = 30;
   CEBA_INVOKE_WRITE(sb, 0, sizeof(uint32_t), &size);
   memcpy(rb->allocation, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 50);
   circular_buffer_read(c, rb, sb);
   circular_buffer_print(c);
   print_buf(rb, 'c');
   printf("--------------------------------------------\n");


   // circular_buffer_write(c, wb, sb);


   CEBA_BUF_FREE_ALL();
   return;
}


void test_basics(void)
{
   char *msg = "hello world my name is Danny";
   printf("hello world!\n");
   buf_t *b = CEBA_BUFREQ(10, 10);
   buf_t *c = CEBA_BUFREQ(20, 8);
   buf_t *d = CEBA_BUFREQ(5, 5);
   
   CEBA_INVOKE_WRITE(b, 0, 10, msg);
   CEBA_INVOKE_WRITE(c, 0, 20, msg);
   CEBA_INVOKE_WRITE(c, 10, 10, msg);
   CEBA_INVOKE_WRITE(d, 0, 5, msg);


   CEBA_PRINT_BUFS('c');
   CEBA_PRINT_BUFS('b');

   int32_t ia = (int32_t) 'a';
   int32_t ib = 1;

   CEBA_INVOKE_WRITE(b, 0, 4, &ia);
   CEBA_INVOKE_WRITE(c, 0, 4, &ib);

   CEBA_INVOKE_ADD_I32(d, b, c);

   int32_t ans = *((int32_t*)d->allocation);

   printf("%d\n", ans);
   printf("%c\n", (char)ans);

   CEBA_PRINT_BUFS('b');


   CEBA_BUF_FREE_ALL();
   return;
}
