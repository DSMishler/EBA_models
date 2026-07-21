#include <unistd.h>
#include "eba.h"

struct global_data
{
   uint64_t nopls; // number of op loaders (will be a uint64_t)
   op_loader_t **opls; // op loaders
   uint64_t nfrargs; // number of args to free (will be a uint64_t)
   void **frargs; // args that need freed
};

typedef struct global_data global_data_t;
