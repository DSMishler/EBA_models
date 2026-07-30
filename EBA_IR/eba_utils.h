#pragma once
#include "prog1_glob.h"
#include "eba.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void free_later(global_data_t*, void*);
void *dl_loader_voidvoidstar_withchecks(void (**func)(void*), char *function_file, char *raw_name);
op_loader_t * opl_init(char *fname, char *op_name);
void check_eba_assumptions(void);
void *get_eba_arg(void *eba_args, int which_arg);
void set_eba_arg(void *eba_args, int which_arg, void *value);
void *init_eba_arg(int nargs);
