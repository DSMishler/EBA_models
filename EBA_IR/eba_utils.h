#pragma once
#include "prog1_glob.h"
#include "eba.h"



void free_later(global_data_t*, void*);
void *dl_loader_voidvoidstar_withchecks(void (**func)(void*), char *function_file, char *raw_name);
op_loader_t * opl_init(char *fname, char *op_name);
