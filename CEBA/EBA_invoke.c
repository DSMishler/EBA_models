#include "EBA_invoke.h"
#include "EBA_buf.h"
#include <string.h>
#include <stdint.h>


void CEBA_INVOKE_WRITE(buf_t *dest, int offset, int len, void *payload)
{
   void *mdest = (void*)((unsigned long int)(dest->allocation) + offset);
   void *msrc = payload;
   memcpy(mdest, msrc, len);
}

void CEBA_INVOKE_ADD_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation)= ia+ib;
}
