#include <unistd.h>
#include "eba.h"

struct global_data
{
   uint64_t nopls; // number of op loaders (will be a uint64_t)
   void **opls; // op loaders
   void *b;
   void *c;
   void *d;
};

typedef struct global_data global_data_t;
