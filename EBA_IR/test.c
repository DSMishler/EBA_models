#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "reader.h"
#include "interpreter.h"
#include "tools/glfw/include/GLFW/glfw3.h"

extern void (*eba_states[16])(void*);
extern void *eba_args[16];

void test_solofile(char *);

int main(void)
{
   GLFWwindow* window = glfwCreateWindow(1000, 1000, "MyWindow", NULL, NULL);
   glfwMakeContextCurrent(window);
   pthread_mutex_init(&interpreter_lock, NULL);
   // printf("EBA tester\n");
   test_solofile("examples/streaming_test/STARTER.EIR");
   pthread_mutex_destroy(&interpreter_lock);
   return 0;
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
   EBA_run(NULL);
}
