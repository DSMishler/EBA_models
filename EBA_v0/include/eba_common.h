#pragma once

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

void load_op(void *arg);

void eba_op(void *arg);


#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void *dl_loader_voidvoidstar_withchecks(void (**func)(void*), char *function_file, char *raw_name);
op_loader_t * opl_init(char *fname, char *op_name);
void opl_destroy(op_loader_t *opl);
void check_eba_assumptions(void);
void *get_eba_arg(void *eba_args, int which_arg);
void set_eba_arg(void *eba_args, int which_arg, void *value);
void *init_eba_arg(int nargs);
void free_eba_arg(void *eba_args);
