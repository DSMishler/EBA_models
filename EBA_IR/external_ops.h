struct op_loader
{
   void (*fn)(void*);// the actual function via indirection
                     // NOTE: this MUST be the first member of the structure.
   char *fname;      // where to find the file containing the op
   char *op_name;    // what said op is called in the file
   void *handler;    // the handler alloc-ed by dlopen. Can be freed later.
   // we can reset this data structure later by reloading the loader and
   // unloading the handler. This need not be an additional data structure
   // entry. It can be done via a separate operation.
};
typedef struct op_loader op_loader_t;
