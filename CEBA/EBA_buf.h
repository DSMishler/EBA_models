struct CEBA_buf
{
   struct CEBA_buf *prev;
   struct CEBA_buf *next;
   void *allocation;
   unsigned int size; // 0 is a reserved value for infinite size
   unsigned int exp;  // 0 is a reserved value for never expiring
};
typedef struct CEBA_buf buf_t;

void print_buf_list(buf_t *head);

buf_t * alloc_buf(int size, int exp);
void free_buf(buf_t *b);

buf_t * add_buf_to_list(buf_t *head, buf_t *addme);
buf_t * remove_buf_from_list(buf_t *head, buf_t *rmvme);
void dealloc_list(buf_t *head);
