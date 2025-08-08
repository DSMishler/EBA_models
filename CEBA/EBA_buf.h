#ifndef _INC_EBA_BUF_
#define _INC_EBA_BUF_

struct CEBA_buf
{
   struct CEBA_buf *prev;
   struct CEBA_buf *next;
   void *allocation;
   unsigned int size; // 0 is a reserved value for infinite size
   unsigned int exp;  // 0 is a reserved value for never expiring
};
typedef struct CEBA_buf buf_t;

void print_buf(buf_t *b, char contents);

void CEBA_PRINT_BUFS(char contents);
buf_t * CEBA_BUFREQ(int size, int exp);
void CEBA_BUF_DEALLOC(buf_t *b);
void CEBA_BUF_FREE_ALL(void);
#endif
