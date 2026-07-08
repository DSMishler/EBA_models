struct op_loader
{
   void (*fn)(void*);
   char *fname;   // where to find the file containing the op
   char *op_name; // what said op is called in the file
};
typedef struct op_loader op_loader_t;


op_loader_t eba_ops[10];
