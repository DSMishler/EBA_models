#include "EBA_buf.h"
#include "EBA_invoke.h"

#include <stdio.h>
#include <stdint.h>

void circular_buffer_print(buf_t *c)
{
   buf_t **buf_data = (buf_t **)c->allocation;

   buf_t *data_buf = buf_data[0];
   buf_t *size_buf = buf_data[1];
   buf_t *head_buf = buf_data[2];
   buf_t *tail_buf = buf_data[3];
   buf_t *full_buf = buf_data[4];

   printf("circular buffer at %lx\n", (uint64_t)c);
   printf("size: %d\n", *(uint32_t*)size_buf->allocation);
   printf("head: %d\n", *(uint32_t*)head_buf->allocation);
   printf("tail: %d\n", *(uint32_t*)tail_buf->allocation);
   printf("full: %d\n", *(uint32_t*)full_buf->allocation);
   printf("contents:\n");
   int head = *(uint32_t*)head_buf->allocation;
   int tail = *(uint32_t*)tail_buf->allocation;
   int i;
   for (i = 0; i < *(uint32_t*)size_buf->allocation; i++)
   {
      printf("%c", ((char*)data_buf->allocation)[i]);
   }
   printf("\n");
   for (i = 0; i < *(uint32_t*)size_buf->allocation; i++)
   {
      if (i == head && i != tail)
      {
         printf("h");
      }
      else if (i == tail && i != head)
      {
         printf("t");
      }
      else if (i == tail && i == head)
      {
         printf("*");
      }
      else
      {
         printf(" ");
      }
   }
   printf("\n");
}

buf_t* circular_buffer_init(int size)
{
   buf_t *c = CEBA_BUFREQ(5*sizeof(buf_t*), 10);


   buf_t *data_buf = CEBA_BUFREQ(size, 10);
   buf_t *size_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *head_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *tail_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *full_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);

   CEBA_INVOKE_WRITE(c, 0*sizeof(buf_t*), sizeof(buf_t*), &data_buf);
   CEBA_INVOKE_WRITE(c, 1*sizeof(buf_t*), sizeof(buf_t*), &size_buf);
   CEBA_INVOKE_WRITE(c, 2*sizeof(buf_t*), sizeof(buf_t*), &head_buf);
   CEBA_INVOKE_WRITE(c, 3*sizeof(buf_t*), sizeof(buf_t*), &tail_buf);
   CEBA_INVOKE_WRITE(c, 4*sizeof(buf_t*), sizeof(buf_t*), &full_buf);

   // TODO: this is sloppy. Fix this and the "one" down below
   uint32_t zero = 0;
   CEBA_INVOKE_WRITE(size_buf, 0, sizeof(uint32_t), &size);
   CEBA_INVOKE_WRITE(head_buf, 0, sizeof(uint32_t), &zero);
   CEBA_INVOKE_WRITE(tail_buf, 0, sizeof(uint32_t), &zero);
   CEBA_INVOKE_WRITE(full_buf, 0, sizeof(uint32_t), &zero);


   return c;
}

void circular_buffer_write(buf_t *c, buf_t *src, buf_t *bytes)
{
   buf_t **buf_data = (buf_t **)c->allocation;

   buf_t *data_buf = buf_data[0];
   buf_t *size_buf = buf_data[1];
   buf_t *head_buf = buf_data[2];
   buf_t *tail_buf = buf_data[3];
   buf_t *full_buf = buf_data[4];


   // Extra work buffers
   buf_t *occ_space_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *cond_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *available_space_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);

   // occupied_space = full*size;
   CEBA_INVOKE_MUL_I32(occ_space_buf, full_buf, size_buf);
   // occupied_space += buf_tail - buf_head
   CEBA_INVOKE_ADD_I32(occ_space_buf, occ_space_buf, tail_buf);
   CEBA_INVOKE_SUB_I32(occ_space_buf, occ_space_buf, head_buf);
   // occupied_space %= buf_size

   CEBA_INVOKE_LT_I32(cond_buf, tail_buf, head_buf);
   // if tail_buf < head_buf
   if (CEBA_TRUE_I32(cond_buf))
   {
      CEBA_INVOKE_ADD_I32(occ_space_buf, occ_space_buf, size_buf);
   }
   CEBA_INVOKE_SUB_I32(available_space_buf, size_buf, occ_space_buf);

   CEBA_INVOKE_LEQ_I32(cond_buf, bytes, available_space_buf);
   if (CEBA_TRUE_I32(cond_buf))
   {
      // enough space to write. Now write.
      // wwsize: working write size
      buf_t *wwsize_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
      CEBA_INVOKE_COPY(wwsize_buf, bytes, sizeof(uint32_t));

      // using the cond buffer as filler here
      CEBA_INVOKE_ADD_I32(cond_buf, bytes, tail_buf);
      CEBA_INVOKE_GT_I32(cond_buf, cond_buf, size_buf);
      // if write_size(bytes) + tail_buf > size_buf
      // then we need to not write over the edge
      if (CEBA_TRUE_I32(cond_buf))
      {
         CEBA_INVOKE_SUB_I32(wwsize_buf, size_buf, tail_buf);
      }

      // first write
      // write to the data_buf at offset tail_buf
      // from the src buf (no offset)
      // of len working write size
      CEBA_INVOKE_OFFCOPY(data_buf, tail_buf, src, NULL, wwsize_buf);

      // add the write size to the tail of the buffer
      CEBA_INVOKE_ADD_I32(tail_buf, tail_buf, wwsize_buf);
      CEBA_INVOKE_MOD_I32(tail_buf, tail_buf, size_buf);

      // now we set wwsize to be the *rest* of the write that needs done
      // this will be zero most of the time, it's only not zero when we
      // wrapped around.
      CEBA_INVOKE_SUB_I32(wwsize_buf, bytes, wwsize_buf);

      // sanity check
      if (*(uint32_t*)tail_buf->allocation != 0)
      {
         // if we didn't write to the end then we better have
         // nothing left to write
         if (*(uint32_t*)wwsize_buf->allocation != 0)
         {
            printf("Sanity check failed... two writes but no wrap-around?\n");
         }
      }

      // use the cond buf as filler here to determine the offset
      // of the src buf
      CEBA_INVOKE_SUB_I32(cond_buf, bytes, wwsize_buf);

      // second write
      // write to the data_buf at offset tail_buf
      //                   (which should be 0 if wwsize isn't zero)
      // from the src buf with offset equal to the length of the first write
      // of len working write size
      CEBA_INVOKE_OFFCOPY(data_buf, tail_buf, src, cond_buf, wwsize_buf);
      // add the write size to the tail of the buffer
      CEBA_INVOKE_ADD_I32(tail_buf, tail_buf, wwsize_buf);
      CEBA_INVOKE_MOD_I32(tail_buf, tail_buf, size_buf);



      // use the occupied_space_buf as a conditional buf
      CEBA_INVOKE_EQ_I32(occ_space_buf, tail_buf, head_buf);
      // TODO: this way of loading in constants is sloppy...
      uint32_t zero = 0;
      CEBA_INVOKE_WRITE(cond_buf, 0, sizeof(uint32_t), &zero);
      CEBA_INVOKE_GT_I32(cond_buf, bytes, cond_buf);
      CEBA_INVOKE_LAND_I32(cond_buf, cond_buf, occ_space_buf);
      // if write_size > 0 and tail_buf == head_buf
      if (CEBA_TRUE_I32(cond_buf))
      {
         uint32_t one = 1;
         CEBA_INVOKE_WRITE(full_buf, 0, sizeof(uint32_t), &one);
      }


      CEBA_BUF_DEALLOC(wwsize_buf);
   }
   else
   {
      printf("refusing write\n");
      // not enough space to write. Abort.
   }

   CEBA_BUF_DEALLOC(available_space_buf);
   CEBA_BUF_DEALLOC(occ_space_buf);
   CEBA_BUF_DEALLOC(cond_buf);
   return;
}

void circular_buffer_read(buf_t *c, buf_t *dest, buf_t *bytes)
{
   buf_t **buf_data = (buf_t **)c->allocation;

   buf_t *data_buf = buf_data[0];
   buf_t *size_buf = buf_data[1];
   buf_t *head_buf = buf_data[2];
   buf_t *tail_buf = buf_data[3];
   buf_t *full_buf = buf_data[4];


   // Extra buffers
   buf_t *occ_space_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
   buf_t *cond_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);

   // occupied_space = full*size;
   CEBA_INVOKE_MUL_I32(occ_space_buf, full_buf, size_buf);
   // occupied_space += buf_tail - buf_head
   CEBA_INVOKE_ADD_I32(occ_space_buf, occ_space_buf, tail_buf);
   CEBA_INVOKE_SUB_I32(occ_space_buf, occ_space_buf, head_buf);
   // occupied_space %= buf_size

   CEBA_INVOKE_LT_I32(cond_buf, tail_buf, head_buf);
   // if tail_buf < head_buf
   if (CEBA_TRUE_I32(cond_buf))
   {
      CEBA_INVOKE_ADD_I32(occ_space_buf, occ_space_buf, size_buf);
   }

   // if our read is not larger than the occupied space
   CEBA_INVOKE_LEQ_I32(cond_buf, bytes, occ_space_buf);
   if (CEBA_TRUE_I32(cond_buf))
   {
      // enough occupied space to read from. Now read.
      // wrsize: working read size
      buf_t *wrsize_buf = CEBA_BUFREQ(sizeof(uint32_t), 10);
      CEBA_INVOKE_COPY(wrsize_buf, bytes, sizeof(uint32_t));

      // using the cond buffer as filler here
      CEBA_INVOKE_ADD_I32(cond_buf, bytes, head_buf);
      CEBA_INVOKE_GT_I32(cond_buf, cond_buf, size_buf);
      // if write_size(bytes) + head_buf > size_buf
      // then we need to not read over the edge
      if (CEBA_TRUE_I32(cond_buf))
      {
         CEBA_INVOKE_SUB_I32(wrsize_buf, size_buf, head_buf);
      }

      // first read
      // read from the data_buf at offset head_buf
      // to the dest buf (no offset)
      // of len working wread size
      CEBA_INVOKE_OFFCOPY(dest, NULL, data_buf, head_buf, wrsize_buf);

      // add the read size to the head of the buffer
      CEBA_INVOKE_ADD_I32(head_buf, head_buf, wrsize_buf);
      CEBA_INVOKE_MOD_I32(head_buf, head_buf, size_buf);

      // now we set wrsize to be the *rest* of the read that needs done
      // this will be zero most of the time, it's only not zero when we
      // wrapped around.
      CEBA_INVOKE_SUB_I32(wrsize_buf, bytes, wrsize_buf);

      // sanity check
      if (*(uint32_t*)head_buf->allocation != 0)
      {
         // if we didn't read to the end then we better have
         // nothing left to read
         if (*(uint32_t*)wrsize_buf->allocation != 0)
         {
            printf("Sanity check failed... two reads but no wrap-around?\n");
         }
      }

      // use the cond buf as filler here to determine the offset
      // of the dest buf
      CEBA_INVOKE_SUB_I32(cond_buf, bytes, wrsize_buf);

      // second read
      // read from the data_buf at offset head_buf
      //                   (which should be 0 if wrsize isn't zero)
      // to the dest buf with offset equal to the length of the first read
      // of len working read size
      CEBA_INVOKE_OFFCOPY(dest, cond_buf, data_buf, head_buf, wrsize_buf);
      // add the reda size to the head of the buffer (again)
      CEBA_INVOKE_ADD_I32(head_buf, head_buf, wrsize_buf);
      CEBA_INVOKE_MOD_I32(head_buf, head_buf, size_buf);


      uint32_t zero = 0;
      CEBA_INVOKE_WRITE(cond_buf, 0, sizeof(uint32_t), &zero);
      CEBA_INVOKE_GT_I32(cond_buf, bytes, cond_buf);
      // if read_size > 0, then the buffer is certainly no longer full
      if (CEBA_TRUE_I32(cond_buf))
      {
         CEBA_INVOKE_WRITE(full_buf, 0, sizeof(uint32_t), &zero);
      }
      // alternatively, we could have checked to see if the buffer was full
      // before and *then* written 0 if it's not full now. But it's simpler
      // just to write that it's not full if any amount of data was read


      CEBA_BUF_DEALLOC(wrsize_buf);
   }
   else
   {
      printf("refusing read\n");
      // not enough space occupied to read. Abort.
   }

   CEBA_BUF_DEALLOC(occ_space_buf);
   CEBA_BUF_DEALLOC(cond_buf);
   return;
}
