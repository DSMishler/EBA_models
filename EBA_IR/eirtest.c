#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "reader.h"
#include "interpreter.h"
// #include <GLFW/glfw3.h>

#include "eba.h"
#include "prog1_glob.h"
#include "eba_utils.h"


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
   global_data_t *gd = get_eba_arg(eba_arg, 1);
   char *dname = get_eba_arg(eba_arg, 2);
   load_dlhandlers("bufreq memop invoke mathop cmp print log scaffold");

   free_later(gd, eba_arg);

   pthread_mutex_init(&interpreter_lock, NULL);
   // printf("EBA tester\n");
   if (!(strcmp(dname, "circ_buf_demo")))
   {
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
   free(dname);
}

void cleanup_demo(void *eba_arg)
{
   global_data_t *gd = get_eba_arg(eba_arg, 1);


   pthread_mutex_destroy(&interpreter_lock);

   free_dlhandlers();


   void *next_eba_arg = init_eba_arg(2);
   set_eba_arg(next_eba_arg, 0, gd->opls[1]);
   set_eba_arg(next_eba_arg, 1, gd);


   eba_args[0] = next_eba_arg; // cleanup
}

void test_solofile(char *fname, void *eba_arg)
{
   global_data_t *gd = get_eba_arg(eba_arg, 1);
   char ***IRcode;
   IRcode = full_read(fname);

   uint64_t *p_w_node = malloc(sizeof(uint64_t));
   *p_w_node = 0;
   uint64_t *p_w_thread = malloc(sizeof(uint64_t));
   *p_w_thread = 0;

   void **eir_arg = init_eba_arg(3);
   set_eba_arg(eir_arg, 0, IRcode);
   set_eba_arg(eir_arg, 1, p_w_node);
   set_eba_arg(eir_arg, 2, p_w_thread);

   op_loader_t *op_loader_eir = opl_init("./libs/EIRtest.so", "run_code");
   op_loader_t *op_loader_run_line = opl_init("./libs/EIRtest.so", "run_line");
   op_loader_t *op_loader_free_IRstate = opl_init("./libs/EIRtest.so", "eba_free_IR_state");
   op_loader_t *op_loader_cleanup_demo = opl_init("./libs/EIRtest.so", "cleanup_demo");
   gd->opls[2] = op_loader_eir;
   gd->opls[3] = op_loader_run_line;
   gd->opls[4] = op_loader_free_IRstate;
   gd->opls[5] = op_loader_cleanup_demo;

   void **arg_buf = init_eba_arg(3);
   set_eba_arg(arg_buf, 0, op_loader_eir);
   set_eba_arg(arg_buf, 1, gd);
   set_eba_arg(arg_buf, 2, eir_arg);

   free_later(gd, arg_buf);

   gd->stored_arg = init_eba_arg(2);
   set_eba_arg(gd->stored_arg, 0, op_loader_cleanup_demo);
   set_eba_arg(gd->stored_arg, 1, gd);

   free_later(gd, gd->stored_arg);

   eba_args[0] = (void*)arg_buf;
}
