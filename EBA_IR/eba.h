#pragma once
#include <stdint.h>
#include "external_ops.h"

#define MAX_THREADS 16

extern void (*eba_states[MAX_THREADS])(void*);
extern void *eba_args[MAX_THREADS];

void * EBA_run(uint64_t w_thread);
void * EBA_run_wrap(void *arg_thread);

void *dl_loader_voidvoidstar(void (**func)(void*), char *function_file, char *raw_name);
void *dl_loader_intvoid(int (**func)(void), char *function_file, char *raw_name);
