#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "reader.h"
#include "interpreter.h"
// #include <GLFW/glfw3.h>

#include "eba.h"
#include "prog1_glob.h"


void test_solofile(char *, void*);

int main(void)
{
   load_dlhandlers("bufreq memop invoke mathop cmp print log scaffold");

   pthread_mutex_init(&interpreter_lock, NULL);
   // printf("EBA tester\n");
   // test_solofile("examples/par_sched_circ_buf/STARTER.EIR");
   test_solofile("examples/streaming_glfw_test/STARTER.EIR", NULL);
   pthread_mutex_destroy(&interpreter_lock);

   free_dlhandlers();

   // glfwTerminate();
   return 0;
}

void run_demo(void *eba_arg)
{
   printf("running run demo!\n");
   global_data_t *gd = *(global_data_t**)((char*) (eba_arg) + sizeof(op_loader_t*));
   char *dname = *(char**)((char*) (eba_arg) + sizeof(op_loader_t*) + sizeof(void*));
   load_dlhandlers("bufreq memop invoke mathop cmp print log scaffold");


   pthread_mutex_init(&interpreter_lock, NULL);
   // printf("EBA tester\n");
   if (!(strcmp(dname, "circ_buf_demo")))
   {
      printf("running circ buf demo!\n");
      test_solofile("examples/par_sched_circ_buf/STARTER.EIR", eba_arg);
   }
   else if (!(strcmp(dname, "stream_demo")))
   {
      test_solofile("examples/streaming_glfw_test/STARTER.EIR", eba_arg);
   }
   else
   {
      printf("I don't understand what demo to load: '%s'\n", dname);
   }
   pthread_mutex_destroy(&interpreter_lock);

   free_dlhandlers();

   free(dname);
   free(eba_arg);

   void *next_eba_arg = malloc(sizeof(op_loader_t*)+sizeof(global_data_t*));
   memcpy(next_eba_arg, &(gd->opls[1]), sizeof(op_loader_t*));


   eba_states[0] = eba_op;
   eba_args[0] = next_eba_arg; // cleanup

}

void test_solofile(char *fname, void *eba_arg)
{
   char ***IRcode;
   IRcode = full_read(fname);

   uint64_t *p_w_node = malloc(sizeof(uint64_t));
   *p_w_node = 0;
   uint64_t *p_w_thread = malloc(sizeof(uint64_t));
   *p_w_thread = 0;

   if(sizeof(op_loader_t*) != sizeof(void*))
   {
      printf("compatibility error!\n");
      // TODO: rewrite so we have no issues like this
      exit(1);
   }
   void **arg_buf = malloc(3*sizeof(void*));
   memcpy(arg_buf, eba_arg, 2*sizeof(void*));
   void **eir_arg = malloc(3*sizeof(void*));
   eir_arg[0] = (void*)IRcode;
   eir_arg[1] = (void*)p_w_node;
   eir_arg[2] = (void*)p_w_thread;
   arg_buf[2] = (void*)eir_arg;


   op_loader_t *op_loader_eir = malloc(sizeof(op_loader_t));
   op_loader_eir->fname = "./libs/EIRtest.so";
   op_loader_eir->op_name = "run_code";
   op_loader_eir->fn = load_op;
   arg_buf[0] = op_loader_eir;




   eba_states[0] = eba_op;
   eba_args[0] = (void*)arg_buf;
   EBA_run_wrap(NULL);
}
