#include "eba_common.h"

#include <assert.h>
#include <stdint.h>

/******************************************************************************/
/*   this first section are functions that belong to CORE eba                 */
/******************************************************************************/

void eba_op(void *arg)
{
   op_loader_t *opl = *((op_loader_t **) arg);
   (opl->fn)(arg);
}

// no checks verison, but a version with checks can be found in eba_utils.c
// and is very useful for debugging. Set it in `load_op` as needed
void *dl_loader_voidvoidstar_nochecks(void (**func)(void*), char *function_file, char *raw_name)
{
   void *object;
   void *handler;

   handler = dlopen(function_file, RTLD_LAZY | RTLD_GLOBAL);
   object = dlsym(handler, raw_name);
   memcpy(func, &object, sizeof(*func));

   return handler;
}

void load_op(void *arg)
{
   op_loader_t *op_ds = *((op_loader_t **)arg);
   // printf("loading op %s\n", op_ds->op_name);
   // to try to keep some sanity here, we will set it to a
   // void* because (for now) the loader spits out non void*s
   // to avoid redundant loads. This may be changed in what
   // is likely an imminent redesign
   op_ds->fn = (void*)0;
   op_ds->handler = dl_loader_voidvoidstar_withchecks(&(op_ds->fn), op_ds->fname, op_ds->op_name);
   // now, it is not guaranteed what kind of structure this was called in.
   // EBA puts this as the initial function pointer for everything.
   // For simplicity, we will not force the user to keep track of whether their
   // op is loaded - we'll just run it after loading quietly
   (*op_ds->fn)(arg);
}

/******************************************************************************/
/*   this is error check, and core EBA *could* run without it                 */
/******************************************************************************/

void *dl_loader_voidvoidstar_withchecks(void (**func)(void*), char *function_file, char *raw_name)
{
   if (*func != (void*)0)
   {
      // function is already loaded. Throw a warning and stop!
      printf("loader called to load %s, but it's already loaded\n", raw_name);
      return NULL;
   }
   void *object;
   char *error;
   void *handler;

   handler = dlopen(function_file, RTLD_LAZY | RTLD_GLOBAL);

   if (!handler)
   {
      printf("%s\n", dlerror());
      return NULL;
   }

   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }

   object = dlsym(handler, raw_name);
   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error! %s\n", error);
   }
   if (object == NULL)
   {
      printf("there is no object!\n");
   }

   memcpy(func, &object, sizeof(*func));

   return handler;
}

/******************************************************************************/
/*   these are utils, and core EBA *could* run without them                   */
/******************************************************************************/

op_loader_t * opl_init(char *fname, char *op_name)
{
   op_loader_t *opl = malloc(sizeof(op_loader_t));
   opl->fname = fname;
   opl->op_name = op_name;
   opl->fn = load_op;
   opl->handler = NULL;
   return opl;
}

void check_eba_assumptions(void)
{
   // necessary error check: EBA ASSUMES these are the same size:
   assert(sizeof(void*) == sizeof(op_loader_t*));
   // assert(sizeof(void*) == sizeof(global_data_t*)); // TODO: check this one too
   assert(sizeof(void*) == sizeof(void**));
   assert(sizeof(void*) == sizeof(char*));
   assert(sizeof(void*) == sizeof(uint64_t*));
}

void *init_eba_arg(int nargs)
{
   return malloc(nargs*sizeof(void*));
}

void *get_eba_arg(void *eba_args, int which_arg)
{
   return ((void**)eba_args)[which_arg];
}

void set_eba_arg(void *eba_args, int which_arg, void *value)
{
   ((void**)eba_args)[which_arg] = value;
}

void free_eba_arg(void *eba_args)
{
   free(eba_args);
}
