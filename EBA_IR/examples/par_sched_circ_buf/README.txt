// 1 - CIRC BUF buffers
   // 00 - CIRC_BUF_READ
   // 08 - CIRC_BUF_WRITE
   // 16 - CIRC_BUF_INIT
   // 24 - CIRC_BUF_FREE
   // 32 - CIRC_BUF_PEEK
   // 40 - CIRC_BUF_UNPACK
   // 48 - CIRC_BUF_REMOVE
   // 56 - CIRC_BUF_ADD_RW_REQUESTS
   // 64 - CIRC_BUF_MAIN
   // 72 - the READ request queue
   // 80 - the WRITE request queue
   // 88 - the circular buffer itself
// 2 - SCHED QUEUE buffers
   // 00 - SCHED_QUEUE_ADD
   // 08 - SCHED_QUEUE_ADD_STACK
   // 16 - SCHED_QUEUE_INIT
   // 24 - SCHED_QUEUE_FREE
   // 32 - SCHED_QUEUE_MAIN
   // 40 - SCHED_QUEUE_PRINT
   // 48 - the schedule queue
   // 56 - explicit prebuilt arg buf for SCHED_QUEUE_MAIN
      // 00 - SCHED_QUEUE_MAIN
      // 08 - the schedule queue (initially unalloc-ed)
      // 16 - explicit arg buf containing code to free all other code/exit buf
// 3 - generalized utility buffers
   // 00 - RELEASE_BUF_STACK
   // 08 - CODE_FREE_STACK
   // 16 - ERROR
   // 24 - bufs_to_free
   // 32 - code_to_free
   // 40 - the parallelism lock


// old args
// args
// 0 - this buf (always)
// 1 - the CIRC BUF buffers
   // 00 - CIRC_BUF_READ
   // 08 - CIRC_BUF_WRITE
   // 16 - CIRC_SCHED_MAIN
   // 24 - CIRC_SCHED_UNPACK
   // 32 - the READ request queue (initially unalloc-ed)
   // 40 - the WRITE request queue (initially unalloc-ed)
// 2 - buffer of the SCHED_QUEUE code buffers (including the call stack one)
   // we'll call one of the CIRC_SCHED code buffers when done
   // MAIN SCHED code buffers
   // 00 - SCHED_QUEUE_ADD
   // 08 - SCHED_QUEUE_ADD_STACK
   // 16 - SCHED_QUEUE_FREE
   // 24 - SCHED_QUEUE_INIT
   // 32 - SCHED_QUEUE_MAIN
   // 40 - SCHED_QUEUE_PEEK
   // 48 - SCHED_QUEUE_PRINT
   // 56 - SCHED_QUEUE_REMOVE
   // 64 - The schedule queue (initially unalloc-ed)
   // 72 - (redundant) prebuilt explicit arg buf to sched_queue_main(built here)
      // 0 - SCHED_QUEUE_MAIN
      // 1 - the schedule queue (initially unalloc-ed)
      // 2 - explicit arg buf containing code for SCHED_QUEUE_FREE
   // 80 - ERROR
   // 88 - SUCCESS
   // 96 - SCHED_QUEUE_PASS_AND_FREE
   // 104- SCHED_QUEUE_PASS_AND_FREE_STACK
