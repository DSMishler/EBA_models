#include <stdio.h>
#include "EBA_buf.h"
#include "EBA_invoke.h"

#include <stdint.h>

int main(void)
{
   char *msg = "hello world my name is Daniel";
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

   int32_t ia = (int32_t) 'a';
   int32_t ib = 1;

   CEBA_INVOKE_WRITE(b, 0, 4, &ia);
   CEBA_INVOKE_WRITE(c, 0, 4, &ib);

   CEBA_INVOKE_ADD_I32(d, b, c);

   int32_t ans = *((int32_t*)d->allocation);

   printf("%d\n", ans);
   printf("%c\n", (char)ans);


   dealloc_list(head);
}
