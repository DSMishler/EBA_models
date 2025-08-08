void circular_buffer_print(buf_t *c);

buf_t* circular_buffer_init(int size);
void circular_buffer_write(buf_t *c, buf_t *src, buf_t *bytes);
void circular_buffer_read(buf_t *c, buf_t *dest, buf_t *bytes);
