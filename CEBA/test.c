#include <stdio.h>
#include "EBA_buf.h"

int main(void)
{
   printf("hello world!\n");
   buf_t *b = alloc_buf(10, 10);
   buf_t *c = alloc_buf(20, 20);
   buf_t *d = alloc_buf(5, 5);
   buf_t *head = add_buf_to_list(NULL, b);
   head = add_buf_to_list(head, c);
   head = add_buf_to_list(head, d);
   print_buf_list(head);

   dealloc_list(head);
}
