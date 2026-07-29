#pragma once
#include <unistd.h>
#include "eba.h"

#define MAX_THREADS 16
extern void *eba_args[MAX_THREADS];

struct global_data
{
   uint64_t nopls; // number of op loaders (will be a uint64_t)
   op_loader_t **opls; // op loaders
   uint64_t nfrargs; // number of args to free (will be a uint64_t)
   void **frargs; // args that need freed
   uint64_t my_thread;
   void **stored_arg; // an arg for eba_arg, indended to be called after EIR
   void **dlclose_after; // a scaffold arg for the dlclose to be called by eba.c
};

typedef struct global_data global_data_t;


void * EBA_run(uint64_t w_thread);
void * EBA_run_wrap(void *arg_thread);
