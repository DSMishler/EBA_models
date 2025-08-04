#ifndef _INC_EBA_INVOKE_
#define _INC_EBA_INVOKE_

#include "EBA_buf.h"
#include <stdint.h>

void CEBA_INVOKE_WRITE(buf_t *dest, int offset, int len, void *payload);
void CEBA_INVOKE_COPY(buf_t *dest, buf_t *src, int len);

// copy with offset
void CEBA_INVOKE_OFFCOPY(
   buf_t *dest,
   buf_t *doffset,
   buf_t *src,
   buf_t *soffset,
   buf_t *len);

void CEBA_INVOKE_ADD_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_SUB_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_MUL_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_MOD_I32(buf_t *dest, buf_t *a, buf_t *b);

void CEBA_INVOKE_GT_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_GEQ_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_LT_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_LEQ_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_EQ_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_NEQ_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_LAND_I32(buf_t *dest, buf_t *a, buf_t *b);
void CEBA_INVOKE_LOR_I32(buf_t *dest, buf_t *a, buf_t *b);

int32_t CEBA_TRUE_I32(buf_t *target);
int32_t CHECK_BUF_VAL_I32(buf_t *taret);
int64_t CHECK_BUF_VAL_I64(buf_t *taret);

#endif
