struct op_loader
{
   void (*fn)(void*);
   char *fname;   // where to find the file containing the op
   char *op_name; // what said op is called in the file
   void *handler;
};
typedef struct op_loader op_loader_t;
