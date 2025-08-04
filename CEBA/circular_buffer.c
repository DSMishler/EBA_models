#include "EBA_buf.h"
#include "EBA_invoke.h"

#include <stdio.h>
#include <stdint.h>

buf_t* circular_buffer_init(int size)
{
   printf("circular buffer init\n");
   buf_t *c = alloc_buf(5*sizeof(buf_t*), 10);


   buf_t *data_buf = alloc_buf(size, 10);
   buf_t *size_buf = alloc_buf(sizeof(uint32_t), 10);
   buf_t *head_buf = alloc_buf(sizeof(uint32_t), 10);
   buf_t *tail_buf = alloc_buf(sizeof(uint32_t), 10);
   buf_t *full_buf = alloc_buf(sizeof(uint32_t), 10);

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


   // TODO: how is this supposed to interact with the list 
   // of currently alloc-ed bufs? I think there should be a
   // CEBA function which allocs bufs and frees them and maintains
   // the linked list on its own
   buf_t *occ_space_buf = alloc_buf(sizeof(uint32_t), 10);
   buf_t *cond_buf = alloc_buf(sizeof(uint32_t), 10);
   buf_t *available_space_buf = alloc_buf(sizeof(uint32_t), 10);

   // occupied_space = full*size;
   CEBA_INVOKE_MUL_I32(occ_space_buf, full_buf, size_buf);
   // occupied_space += buf_tail - buf_head
   CEBA_INVOKE_ADD_I32(occ_space_buf, occ_space_buf, tail_buf);
   CEBA_INVOKE_SUB_I32(occ_space_buf, occ_space_buf, head_buf);
   // occupied_space %= buf_size
   // TODO: is this needed? I think maybe it's not
   // CEBA_INVOKE_MOD_I32(occ_space_buf, occ_space_buf, size_buf);

   CEBA_INVOKE_LT_I32(cond_buf, tail_buf, head_buf);
   // if tail_buf < head_buf
   if (CEBA_TRUE_I32(cond_buf))
   {
      CEBA_INVOKE_ADD_I32(occ_space_buf, occ_space_buf, size_buf);
   }
   CEBA_INVOKE_SUB_I32(available_space_buf, size_buf, occ_space_buf);

   printf("occ space: %d\n", *(uint32_t*)occ_space_buf->allocation);
   CEBA_INVOKE_LEQ_I32(cond_buf, bytes, available_space_buf);
   if (CEBA_TRUE_I32(cond_buf))
   {
      // wwsize: working write size
      buf_t *wwsize_buf = alloc_buf(sizeof(uint32_t), 10);
      CEBA_INVOKE_COPY(wwsize_buf, bytes, sizeof(uint32_t));

      // using the cond buffer as filler here
      CEBA_INVOKE_ADD_I32(cond_buf, bytes, tail_buf);
      printf("cond val: %d\n", *(uint32_t*)cond_buf->allocation);
      CEBA_INVOKE_GT_I32(cond_buf, cond_buf, size_buf);
      printf("cond val: %d\n", *(uint32_t*)cond_buf->allocation);
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
      printf("data buf %lx\n", (uint64_t)data_buf->allocation);
      printf("src buf %lx\n", (uint64_t)src->allocation);
      printf("tail buf %d\n", *(uint32_t*)src->allocation);
      printf("requested bytes: %d\n", *(uint32_t*)bytes->allocation);
      printf("wwize: %d\n", *(uint32_t*)wwsize_buf->allocation);

      CEBA_INVOKE_OFFCOPY(data_buf, tail_buf, src, NULL, wwsize_buf);

      // add the write size to the tail of the buffer
      CEBA_INVOKE_ADD_I32(tail_buf, tail_buf, wwsize_buf);
      CEBA_INVOKE_MOD_I32(tail_buf, tail_buf, size_buf);

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
      // write to the data_buf at offset tail_buf (which should be 0)
      // from the src buf with offset equal to the length of the first write
      // of len working write size
      CEBA_INVOKE_OFFCOPY(data_buf, tail_buf, src, cond_buf, wwsize_buf);
      // add the write size to the tail of the buffer
      CEBA_INVOKE_ADD_I32(tail_buf, tail_buf, wwsize_buf);
      CEBA_INVOKE_MOD_I32(tail_buf, tail_buf, size_buf);



      // use the occupied_space_buf as a conditional buf
      CEBA_INVOKE_EQ_I32(occ_space_buf, tail_buf, head_buf);
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


      free_buf(wwsize_buf);
      // enough space to write. Now write.
   }
   else
   {
      printf("refusing write\n");
      // not enough space to write. Abort.
   }

   free_buf(available_space_buf);
   free_buf(occ_space_buf);
   free_buf(cond_buf);
   return;
}
