#ifndef _INC_EBA_INVOKE_
#define _INC_EBA_INVOKE_

#include "EBA_buf.h"

void CEBA_INVOKE_WRITE(buf_t *dest, int offset, int len, void *payload);
void CEBA_INVOKE_ADD_I32(buf_t *dest, buf_t *a, buf_t *b);

#endif
