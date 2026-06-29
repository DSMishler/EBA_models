#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "reader.h"
#include "interpreter.h"
// #include <GLFW/glfw3.h>

extern void (*eba_states[16])(void*);
extern void *eba_args[16];

void test_solofile(char *);

int main(void)
{
   // if (!glfwInit())
      // return -1;

   // GLFWwindow* window = glfwCreateWindow(1000, 1000, "MyWindow", NULL, NULL);
   // glfwMakeContextCurrent(window);

   load_dlhandlers("bufreq memop invoke mathop cmp print log scaffold");

   pthread_mutex_init(&interpreter_lock, NULL);
   // printf("EBA tester\n");
   // test_solofile("examples/par_sched_circ_buf/STARTER.EIR");
   test_solofile("examples/streaming_glfw_test/STARTER.EIR");
   pthread_mutex_destroy(&interpreter_lock);

   free_dlhandlers();

   // glfwTerminate();
   return 0;
}

void run_demo(void *which)
{
   char *dname = (char*) which;
   load_dlhandlers("bufreq memop invoke mathop cmp print log scaffold");

   pthread_mutex_init(&interpreter_lock, NULL);
   // printf("EBA tester\n");
   if (!(strcmp(dname, "circ_buf_demo")))
   {
      test_solofile("examples/par_sched_circ_buf/STARTER.EIR");
   }
   else if (!(strcmp(dname, "stream_demo")))
   {
      test_solofile("examples/streaming_glfw_test/STARTER.EIR");
   }
   else
   {
      printf("I don't understand what demo to load: '%s'\n", dname);
   }
   pthread_mutex_destroy(&interpreter_lock);

   free_dlhandlers();

   free(dname);

}

void test_solofile(char *fname)
{
   char ***IRcode;
   IRcode = full_read(fname);

   uint64_t *p_w_node = malloc(sizeof(uint64_t));
   *p_w_node = 0;
   uint64_t *p_w_thread = malloc(sizeof(uint64_t));
   *p_w_thread = 0;

   void **arg_buf = malloc(3*sizeof(void*));
   arg_buf[0] = (void*)IRcode;
   arg_buf[1] = (void*)p_w_node;
   arg_buf[2] = (void*)p_w_thread;

   eba_states[0] = &run_code;
   eba_args[0] = (void*)arg_buf;
   EBA_run_wrap(NULL);
}
