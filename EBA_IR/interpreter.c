#include "interpreter.h"
#include "reader.h"

#include <dlfcn.h>

pthread_mutex_t interpreter_lock;

void (*run_bufreq)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_memop)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_invoke)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_mathop)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_cmp)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_print)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_log)(IR_state_t *IRstate, char **line) = (void*)0;
void (*run_scaffold)(IR_state_t *IRstate, char **line) = (void*)0;

void *dlhandlers[MAX_MODULES] = {NULL};

void module_loader(void (**func)(IR_state_t *, char **), char *module_file, char *raw_name)
{
   if (*func != (void*)0)
   {
      // function is already loaded. Throw a warning and stop!
      printf("loader called to load %s, but it's already loaded\n", raw_name);
      return;
   }
   // else:
   char *error;
   void *handler = dlopen(module_file, RTLD_LAZY);
   if (!handler)
   {
      printf("interpreter handler error: %s\n", dlerror());
   }
   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error in the dlopen! '%s'\n", error);
   }
   void *object = dlsym(handler, raw_name);
   error = dlerror();
   if (error != NULL)
   {
      printf("there was an error in the dlsym! '%s'\n", error);
   }
   if (object == NULL)
   {
      printf("there was no object returned!\n");
   }
   // save the module for freeing later
   int i;
   for(i = 0; i < MAX_MODULES; i++)
   {
      if (dlhandlers[i] == NULL)
      {
         dlhandlers[i] = handler;
         break;
      }
   }
   if (i == MAX_MODULES)
   {
      fprintf(stderr, "something went wrong with the number of available modules!\n");
      exit(1);
   }
   memcpy(func, &object, sizeof(*func));
}

void load_dlhandlers(char *line)
{
   char **words = line_to_words(line);
   int i;

   for(i = 0; words[i] != NULL; i++)
   {
      if (samestr(words[i], "bufreq"))
      {
         module_loader(&run_bufreq, "./libs/bufreq.so", "run_bufreq");
      }
      else if (samestr(words[i], "memop"))
      {
         module_loader(&run_memop, "./libs/memop.so", "run_memop");
      }
      else if (samestr(words[i], "invoke"))
      {
         module_loader(&run_invoke, "./libs/invoke.so", "run_invoke");
      }
      else if (samestr(words[i], "mathop"))
      {
         module_loader(&run_mathop, "./libs/mathop.so", "run_mathop");
      }
      else if (samestr(words[i], "cmp"))
      {
         module_loader(&run_cmp, "./libs/cmp.so", "run_cmp");
      }
      else if (samestr(words[i], "print"))
      {
         module_loader(&run_print, "./libs/print.so", "run_print");
      }
      else if (samestr(words[i], "log"))
      {
         module_loader(&run_log, "./libs/log.so", "run_log");
      }
      else if (samestr(words[i], "scaffold"))
      {
         module_loader(&run_scaffold, "./libs/scaffold.so", "run_scaffold");
      }
   }




   for(i = 0; words[i] != NULL; i++)
   {
      free(words[i]);
   }
   free(words);
}

void free_dlhandlers(void)
{
   int i;
   for(i = 0; i < MAX_MODULES; i++)
   {
      if (dlhandlers[i] != NULL)
      {
         dlclose(dlhandlers[i]);
         dlhandlers[i] = NULL;
      }
   }
   // TODO: this is... really sloppy. Consider a rewrite.
   run_bufreq = (void*)0;
   run_memop  = (void*)0;
   run_invoke = (void*)0;
   run_mathop = (void*)0;
   run_cmp    = (void*)0;
   run_print  = (void*)0;
   run_log    = (void*)0;
   run_scaffold = (void*)0;
}

int confirm_first_word(char **line, char *word)
{
   char *first = line[0];
   if (first == NULL)
   {
      fprintf(stderr, "error: empty line passed into confirm_first_word\n");
      return 0;
   }
   if (!samestr(first, word))
   {
      fprintf(stderr, "error: expected word %s but instead see word %s\n", word, first);
      return 0;
   }
   else
   {
      return 1;
   }
}

int match_second_word(char **line, char *word)
{
   char *second = line[1];
   if (second == NULL)
   {
      fprintf(stderr, "error: one word line passed into confirm_second_word\n");
      return 0;
   }
   return samestr(second, word);
}

int parse_variable(char *word)
{
   if (word == NULL)
   {
      fprintf(stderr, "error: NULL word passed to parse_variable\n");
      return -1;
   }
   if (word[0] != 'V')
   {
      fprintf(stderr, "error: expected a variable (e.g. V14, V09, V1), got %s\n", word);
      return -1;
   }
   return atoi(word+1);
}

void* parse_var_buf(char *word, IR_state_t *IRstate)
{
   if (word == NULL)
   {
      fprintf(stderr, "error: NULL word passed to parse_variable\n");
      return NULL;
   }
   void *retval;
   if (word[0] == 'V')
   {
      // then it's a variable
      int which_var = parse_variable(word);
      assert(which_var >= 0 && which_var < IR_STATE_SIZE);
      retval = (void*)IRstate->vars[which_var];
   }
   else if (word[0] == '&')
   {
      if (word[strlen(word)-1] == 'f')
      {
         // it's a float, consider it a double precision float.
         char *newstr = malloc(strlen(word)+1);
         strcpy(newstr, word);
         newstr[strlen(word)] = '\0';
         // TODO: it's sloppy to change caller memory. Change this.
         char *strend;
         double myval = strtod(newstr+1, &strend);
         assert(strend != NULL);
         retval = malloc(sizeof(uint64_t));
         *(double*)retval = myval;
         free(newstr);
      }
      else
      {
         char *strend;
         uint64_t myval = strtoull(word+1, &strend, 10);
         assert(strend != NULL);
         retval = malloc(sizeof(uint64_t));
         *(uint64_t*)retval = myval;
      }
   }
   else
   {
      fprintf(stderr, "error: expected a variable or '&' buf (e.g. V14, V09, V1, &5), got %s\n", word);
      return NULL;
   }
   return retval;
}

void buf_free_if_shorthand(void *buf, char *word)
{
   if (word[0] == '&')
   {
      // then we alloc-ed the buf earlier and should free it
      free(buf);
   }
   else
   {
      ; // if it was a variable, no need to do any freeing
   }
}

// TODO: change all of this to int64s
uint64_t parse_literal(char *word)
{
   if (word == NULL)
   {
      fprintf(stderr, "error: NULL word passed to parse_literal\n");
      exit(1);
   }
   if (word[0] != '@')
   {
      fprintf(stderr, "error: expected a literal (e.g. @14, @09, @1), got %s\n", word);
      exit(1);
   }
   char *strend;
   uint64_t retval = strtoull(word+1, &strend, 10);
   assert(strend != NULL);
   return retval;
}


void var_errmsg(char *func, int line)
{
   fprintf(stderr, "%s on line %d: variable out of range\n", func, line+1);
   exit(1);
}


// NOTE: not threadsafe inherently. Wrap in locks
int get_avail_w_thread(void)
{
   int i;
   for (i = 0; i < MAX_THREADS; i++)
   {
      if (eba_states[i] == (void*)0)
      {
         break;
      }
   }
   int w_thread = i;
   if (w_thread >= MAX_THREADS)
   {
      return -1;
   }
   return w_thread;
}

// NOTE: not threadsafe inherently. Wrap in locks
int thread_is_avail(int w_thread)
{
   if (w_thread >= MAX_THREADS || w_thread < 0)
   {
      return 0;
   }
   if (eba_states[w_thread] == (void*)0)
   {
      return 1;
   }
   return 0;
}





void run_noop(IR_state_t *IRstate)
{
   IRstate->next_line += 1;
}


void run_line(void* lcl_eba_arg)
{
   IR_state_t *IRstate = (IR_state_t*)lcl_eba_arg;
   char **line = IRstate->code_buf[IRstate->next_line];
   if (line == NULL)
   {
      // printf("thread %lu detecting termination\n", IRstate->w_thread);
      eba_states[IRstate->w_thread] = &eba_free_IR_state;
      // do no work, and return here
      return;
   }

   if(0) // print
   {
      printf("t%lu running line %d:", IRstate->w_thread, IRstate->next_line+1);
      for(int i = 0; line[i] != NULL; i++)
      {
         printf(" %s", line[i]);
      }
      printf("\n");
   }

   if (line[0] == NULL)
   {
      // it's a no-op
      run_noop(IRstate);
   }
   else if (is_label(line[0]))
   {
      run_noop(IRstate);
      // printf("flag: detected a label (no-op) on line %d\n", IRstate->next_line);
   }

   // now check against all known functions
   else if (samestr(line[0], "BUFREQ"))
   {
      if (run_bufreq == (void*)0)
      {
         printf("error: module BUFREQ not loaded\n");
         return;
      }
      run_bufreq(IRstate, line);
   }
   else if (samestr(line[0], "MEMOP"))
   {
      if (run_memop == (void*)0)
      {
         printf("error: module MEMOP not loaded\n");
         return;
      }
      run_memop(IRstate, line);
   }
   else if (samestr(line[0], "INVOKE"))
   {
      if (run_invoke == (void*)0)
      {
         printf("error: module INVOKE not loaded\n");
         return;
      }
      run_invoke(IRstate, line);
   }
   else if (samestr(line[0], "MATHOP"))
   {
      if (run_mathop == (void*)0)
      {
         printf("error: module MATHOP not loaded\n");
         return;
      }
      run_mathop(IRstate, line);
   }
   else if (samestr(line[0], "CMP"))
   {
      if (run_cmp == (void*)0)
      {
         printf("error: module CMP not loaded\n");
         return;
      }
      run_cmp(IRstate, line);
   }
   else if (samestr(line[0], "PRINT"))
   {
      if (run_print == (void*)0)
      {
         printf("error: module PRINT not loaded\n");
         return;
      }
      run_print(IRstate, line);
   }
   else if (samestr(line[0], "LOG"))
   {
      if (run_log == (void*)0)
      {
         printf("error: module LOG not loaded\n");
         return;
      }
      run_log(IRstate, line);
   }
   else if (samestr(line[0], "SCAFFOLD"))
   {
      if (run_scaffold == (void*)0)
      {
         printf("error: module SCAFFOLDnot loaded\n");
         return;
      }
      // printf("running scaffold!\n");
      // printf("running scaffold fr!\n");
      run_scaffold(IRstate, line);
      // dlclose(handler);
      // printf("all is well!\n");

   }
   else
   {
      fprintf(stderr, "unknown command '%s' on line %d\n",
              line[0], IRstate->next_line);
      exit(1);
   }
}

void run_code(void* lcl_eba_arg)
{
   char ****arg_buf = (char****)lcl_eba_arg;
   char*** code_buf = ((char****)(lcl_eba_arg))[0];
   uint64_t* p_w_node = ((uint64_t**)(lcl_eba_arg))[1];
   uint64_t w_node = *p_w_node;
   uint64_t* p_w_thread = ((uint64_t**)(lcl_eba_arg))[2];
   uint64_t w_thread = *p_w_thread;
   IR_state_t *IRstate = init_IR_state();

   // Note: we do NOT zero out all the other vars.
   // Advocate for zeroing: not doing this may possibly add a vulnerability
   // Advocate against: doing this makes things less minimal
   IRstate->w_node = w_node;
   IRstate->w_thread = w_thread;
   IRstate->vars[0] = (int64_t) (arg_buf);
   IRstate->next_line = 0;
   IRstate->code_buf = code_buf;

   eba_args[IRstate->w_thread] = (void*)(IRstate);
   eba_states[IRstate->w_thread] = &run_line;
}

IR_state_t * init_IR_state(void)
{
   IR_state_t *IRstate;
   IRstate = malloc(sizeof(IR_state_t));
   IRstate->w_thread = -1;
   // NOTE: this is NOT in the standard to give you calloc-ed memory,
   //       only malloc-ed is guaranteed
   IRstate->vars = calloc(IR_STATE_SIZE, sizeof(int64_t));
   IRstate->next_line = 0;
   IRstate->code_buf = NULL;


   return IRstate;
}

void print_IR_state(IR_state_t *IRstate)
{
   printf("printing IR state:\n");
   int i;
   for(i = 0; i < IR_STATE_SIZE; i++)
   {
      printf("V%02d: %lx\n", i, IRstate->vars[i]);
   }
}

void eba_free_IR_state(void* lcl_eba_arg)
{
   IR_state_t *IRstate = (IR_state_t *)lcl_eba_arg;
   eba_args[IRstate->w_thread] = NULL;
   eba_states[IRstate->w_thread] = (void*)0;
   free_IR_state(IRstate);
}

void free_IR_state(IR_state_t *IRstate)
{
   if (IRstate == NULL)
   {
      printf("warning, IRstate passed as NULL\n");
      return;
   }

   free(IRstate->vars);
   free(IRstate);
}
