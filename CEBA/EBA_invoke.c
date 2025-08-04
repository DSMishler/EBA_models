#include "EBA_invoke.h"
#include "EBA_buf.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>


void CEBA_INVOKE_WRITE(buf_t *dest, int offset, int len, void *payload)
{
   void *mdest = (void*)((unsigned long int)(dest->allocation) + offset);
   void *msrc = payload;
   memcpy(mdest, msrc, len);
}

void CEBA_INVOKE_COPY(buf_t *dest, buf_t *src, int len)
{
   void *mdest = dest->allocation;
   void *msrc = src->allocation;
   memcpy(mdest, msrc, len);
}

void CEBA_INVOKE_OFFCOPY(
   buf_t *dest,
   buf_t *doffset,
   buf_t *src,
   buf_t *soffset,
   buf_t *len)
{
   void *mdest = dest->allocation;
   void *msrc = src->allocation;
   printf("old ptrs: 0x%lx 0x%lx\n", (uint64_t)mdest, (uint64_t)msrc);
   if (doffset != NULL)
   {
      mdest = (void*)((uint64_t) mdest + *(uint32_t*) doffset->allocation);
   }
   if (soffset != NULL)
   {
      msrc = (void*)((uint64_t) msrc + *(uint32_t*) soffset->allocation);
   }
   int l = *(uint32_t*)(len->allocation);
   printf("new ptrs: 0x%lx 0x%lx\n", (uint64_t)mdest, (uint64_t)msrc);
   printf("and len: %d\n", l);
   memcpy(mdest, msrc, l);
}

void CEBA_INVOKE_ADD_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation)= ia+ib;
}

void CEBA_INVOKE_SUB_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation)= ia-ib;
}

void CEBA_INVOKE_MUL_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation)= ia*ib;
}

void CEBA_INVOKE_MOD_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation)= ia%ib;
}

void CEBA_INVOKE_GT_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia > ib);
}

void CEBA_INVOKE_GEQ_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia >= ib);
}

void CEBA_INVOKE_LT_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia < ib);
}

void CEBA_INVOKE_LEQ_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia <= ib);
}

void CEBA_INVOKE_EQ_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia == ib);
}

void CEBA_INVOKE_NEQ_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia != ib);
}

void CEBA_INVOKE_LAND_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia && ib);
}

void CEBA_INVOKE_LOR_I32(buf_t *dest, buf_t *a, buf_t *b)
{
   int32_t ia = *((int32_t*)a->allocation);
   int32_t ib = *((int32_t*)b->allocation);

   *((int32_t*)dest->allocation) = (ia || ib);
}

// helper
int32_t CEBA_TRUE_I32(buf_t *target)
{
   return *((int32_t*)target->allocation);
}

int32_t CHECK_BUF_VAL_I32(buf_t *target)
{
   return *((int32_t*)target->allocation);
}

int64_t CHECK_BUF_VAL_I64(buf_t *target)
{
   return *((int64_t*)target->allocation);
}
