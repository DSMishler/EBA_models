#include "interpreter.h"
#include "reader.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

void run_scaffold(IR_state_t *IRstate, char **line)
{
   // printf("actual scaffold function\n");
   assert(confirm_first_word(line, "SCAFFOLD"));

   if (match_second_word(line, "SYSTEM"))
   {
      assert(line[2] != NULL);
      system(line[2]);
   }
   else if (match_second_word(line, "CODEREAD"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD CODEREAD", IRstate->next_line);
      }

      assert(line[3] != NULL);
      IRstate->vars[var_dest] = (uint64_t) full_read(line[3]);
   }
   else if (match_second_word(line, "CODEFREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD CODEFREE", IRstate->next_line);
      }
      full_free((char***)IRstate->vars[var_dest]);
   }
   else if (match_second_word(line, "TERMINATE_WITH_CODEFREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD TERMINATE_WITH_CODEFREE", IRstate->next_line);
      }
      full_free((char***)IRstate->vars[var_dest]);
      eba_states[IRstate->w_thread] = &eba_free_IR_state;
   }
   else if (match_second_word(line, "P_SEM_INIT"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_INIT", IRstate->next_line);
      }
      void *psem = malloc(sizeof(sem_t));
      sem_init(psem, 0, 0);
      IRstate->vars[var_dest] = (int64_t) psem;
   }
   else if (match_second_word(line, "P_SEM_WAIT"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_WAIT", IRstate->next_line);
      }
      void *psem = (void*) IRstate->vars[var_dest];
      sem_wait(psem);
   }
   else if (match_second_word(line, "P_SEM_POST"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_POST", IRstate->next_line);
      }
      void *psem = (void*) IRstate->vars[var_dest];
      sem_post(psem);
   }
   else if (match_second_word(line, "P_SEM_FREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_SEM_FREE", IRstate->next_line);
      }
      void *psem = (void*) IRstate->vars[var_dest];
      sem_destroy(psem);
      free(psem);
   }
   else if (match_second_word(line, "P_LOCK_INIT"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_INIT", IRstate->next_line);
      }
      void *plock = malloc(sizeof(pthread_mutex_t));
      pthread_mutex_init(plock, NULL);
      IRstate->vars[var_dest] = (int64_t) plock;
   }
   else if (match_second_word(line, "P_LOCK_ACK"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_ACK", IRstate->next_line);
      }
      void *plock = (void*) IRstate->vars[var_dest];
      pthread_mutex_lock(plock);
   }
   else if (match_second_word(line, "P_LOCK_REL"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_REL", IRstate->next_line);
      }
      void *plock = (void*) IRstate->vars[var_dest];
      pthread_mutex_unlock(plock);
   }
   else if (match_second_word(line, "P_LOCK_FREE"))
   {
      int var_dest = parse_variable(line[2]);
      if (var_dest < 0 || var_dest >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD P_LOCK_FREE", IRstate->next_line);
      }
      void *plock = (void*) IRstate->vars[var_dest];
      pthread_mutex_destroy(plock);
      free(plock);
   }
   else if (match_second_word(line, "PTHREAD_SPAWN_HEAVY"))
   {
      pthread_mutex_lock(&interpreter_lock);
      void *arg_buf = parse_var_buf(line[2], IRstate);
      void **eba_arg = malloc(3*sizeof(void*));
      eba_arg[0] = NULL;
      eba_arg[1] = NULL;
      eba_arg[2] = arg_buf;

      uint64_t *p_w_thread = ((uint64_t **)arg_buf)[2];
      uint64_t w_thread = *p_w_thread;
      if (w_thread >= MAX_THREADS)
      {
         fprintf(stderr, "error: thread %lu not allowed (max is %d). Stop.\n",
                  w_thread, MAX_THREADS-1);
         exit(1);
      }

      if (!(thread_is_avail(w_thread)))
      {
         fprintf(stderr, "error: thread %lu appears to be taken!. Stop.\n", w_thread);
         exit(1);
      }

      void *targ = malloc(sizeof(uint64_t));
      *((uint64_t*)targ) = w_thread; // will be free-ed later
      
      pthread_t tids[1];

      eba_states[w_thread] = &run_code;
      eba_args[w_thread] = eba_arg;

      // printf("creating thread with targ pointing to %lu\n", w_thread);
      pthread_create(tids, NULL, EBA_run_wrap, targ);
      pthread_detach(tids[0]);


      buf_free_if_shorthand(arg_buf, line[2]);
      pthread_mutex_unlock(&interpreter_lock);
   }
   else if (match_second_word(line, "PTHREAD_GET_TID"))
   {
      void *tid_buf = parse_var_buf(line[2], IRstate);
      
      *((uint64_t*)tid_buf) = IRstate->w_thread;

      buf_free_if_shorthand(tid_buf, line[2]);
   }
   else if (match_second_word(line, "GET_NODEID"))
   {
      void *tid_buf = parse_var_buf(line[2], IRstate);
      
      *((uint64_t*)tid_buf) = IRstate->w_node;

      buf_free_if_shorthand(tid_buf, line[2]);
   }
   else if (match_second_word(line, "PTHREAD_GET_AVAIL"))
   {
      pthread_mutex_lock(&interpreter_lock);
      void *avl_buf = parse_var_buf(line[2], IRstate);
      
      uint64_t w_thread = get_avail_w_thread();
      if (w_thread == (uint64_t)(-1))
      {
         fprintf(stderr, "no available thread! You're probably going to segfault.\n");
      }

      *((uint64_t*)avl_buf) = w_thread;

      buf_free_if_shorthand(avl_buf, line[2]);
      pthread_mutex_unlock(&interpreter_lock);
   }
   else if (match_second_word(line, "GLFW_INIT"))
   {
      int var_window = parse_variable(line[2]);
      if (var_window < 0 || var_window >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_INIT", IRstate->next_line);
      }

      if (!glfwInit())
      {
         fprintf(stderr, "ERROR: GLFW3 init failed!\n");
         IRstate->vars[var_window] = 0; // NULL
         IRstate->next_line += 1;
         return;
      }

      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      GLFWwindow *window = glfwCreateWindow(800, 600, "EIRstream", NULL, NULL);

      if (!window)
      {
         fprintf(stderr, "ERROR: GLFW3 window creation failed!\n");
         IRstate->vars[var_window] = 0; // NULL
         IRstate->next_line += 1;
         return;
      }

      glfwMakeContextCurrent(window);
      glfwSwapInterval(1);

      int version_glad = gladLoadGL(glfwGetProcAddress);
      if (version_glad == 0)
      {
         fprintf(stderr, "ERROR: failed to init OpenGL!\n");
         IRstate->vars[var_window] = 0; // NULL
         IRstate->next_line += 1;
         return;
      }

      glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

      IRstate->vars[var_window] = (int64_t) window;
   }
   else if (match_second_word(line, "GLFW_TERMINATE"))
   {
      glfwTerminate();
   }
   else if (match_second_word(line, "GLFW_WINDOWSHOULDCLOSE"))
   {
      // following the dest, src convention
      int var_shouldclose = parse_variable(line[2]);
      int var_window = parse_variable(line[3]);
      if (var_shouldclose < 0 || var_shouldclose >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_WINDOWSHOULDCLOSE", IRstate->next_line);
      }
      if (var_window < 0 || var_window >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_WINDOWSHOULDCLOSE", IRstate->next_line);
      }
      
      GLFWwindow *window = (GLFWwindow *) IRstate->vars[var_window];
      int64_t *shouldclose = (int64_t*) IRstate->vars[var_shouldclose];
      *shouldclose = (int64_t) glfwWindowShouldClose(window);
   }
   else if (match_second_word(line, "GLFW_POLLEVENTS"))
   {
      glfwPollEvents();
   }
   else if (match_second_word(line, "GLFW_CLEAR"))
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }
   else if (match_second_word(line, "GLFW_SWAPBUFFERS"))
   {
      int var_window = parse_variable(line[2]);
      if (var_window < 0 || var_window >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_SWAPBUFFERS", IRstate->next_line);
      }
      GLFWwindow *window = (GLFWwindow *) IRstate->vars[var_window];
      glfwSwapBuffers(window);

   }
   else if (match_second_word(line, "GLFW_MAKEVAO"))
   {
      int var_vao = parse_variable(line[2]);
      int var_data = parse_variable(line[3]);
      if (var_vao < 0 || var_vao >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_SWAPBUFFERS", IRstate->next_line);
      }
      if (var_data < 0 || var_data >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_SWAPBUFFERS", IRstate->next_line);
      }

      double *data = (double *) IRstate->vars[var_data];

      GLuint vbo = 0;
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, 9*sizeof(double), data, GL_STATIC_DRAW);

      // for(int i = 0; i < 3; i++)
      // {
         // printf("points[%d]: %lf %lf %lf\n", i, data[3*i+0], data[3*i+1], data[3*i+2]);
      // }
      // printf("\n");

      GLuint vao = 0;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, NULL);

      IRstate->vars[var_vao] = (int64_t) vao;
   }
   else if (match_second_word(line, "GLFW_MAKESHADERPROG"))
   {
      int var_prog = parse_variable(line[2]);
      if (var_prog < 0 || var_prog >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_MAKESHADERPROG", IRstate->next_line);
      }
      long int flen;
      FILE *fp;
      int chars_read;

      assert(line[3] != NULL);

      fp = fopen(line[3], "rb"); // TODO: check with just "r" too, should be fine
      if (fp == NULL)
      {
         fprintf(stderr, "error: '%s' not found\n", line[3]);
         exit(1);
      }
      fseek(fp, 0, SEEK_END);
      flen = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      char *vertex_shader = malloc(flen+1);
      chars_read = fread(vertex_shader, 1, flen, fp);
      if (chars_read != flen)
      {
         fprintf(stderr, "error in reading vertex shader!\n");
         exit(1);
      }
      fclose(fp);
      vertex_shader[flen] = '\0';

      assert(line[4] != NULL);

      fp = fopen(line[4], "rb"); // TODO: check with just "r" too, should be fine
      if (fp == NULL)
      {
         fprintf(stderr, "error: '%s' not found\n", line[4]);
         exit(1);
      }
      fseek(fp, 0, SEEK_END);
      flen = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      char *fragment_shader = malloc(flen+1);
      chars_read = fread(fragment_shader, 1, flen, fp);
      if (chars_read != flen)
      {
         fprintf(stderr, "error in reading fragment shader!\n");
         exit(1);
      }
      fclose(fp);
      fragment_shader[flen] = '\0';

      // printf("%s\n", vertex_shader);
      // printf("%s\n", fragment_shader);

      GLuint vs = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vs, 1, (const char**) &vertex_shader, NULL);
      glCompileShader(vs);

      GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fs, 1, (const char**) &fragment_shader, NULL);
      glCompileShader(fs);

      GLuint shader_program = glCreateProgram();
      glAttachShader(shader_program, fs);
      glAttachShader(shader_program, vs);
      glLinkProgram(shader_program);

      IRstate->vars[var_prog] = (int64_t) shader_program;
      // printf("value of shader program? %d\n", shader_program);

      free(vertex_shader);
      free(fragment_shader);
   }
   else if (match_second_word(line, "GLFW_USESHADERPROG"))
   {
      int var_prog = parse_variable(line[2]);
      if (var_prog < 0 || var_prog >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_USESHADERPROG", IRstate->next_line);
      }

      GLuint shader_program = (GLuint) IRstate->vars[var_prog];
      // printf("size of GLuint? %lu\n", sizeof(GLuint));
      // printf("value of shader program? %u\n", shader_program);
      glUseProgram(shader_program);
   }
   else if (match_second_word(line, "GLFW_DRAWVAOTRIANGLES"))
   {
      int var_vao = parse_variable(line[2]);
      if (var_vao < 0 || var_vao >= IR_STATE_SIZE)
      {
         var_errmsg("SCAFFOLD GLFW_DRAWVAOTRIANGLES", IRstate->next_line);
      }

      GLuint vao = (GLuint) IRstate->vars[var_vao];
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDrawArrays(GL_POINTS, 0, 3);
   }
   else
   {
      fprintf(stderr, "error: option %s does not exist for SCAFFOLD\n", line[1]);
   }
   
   // printf("exiting scaffold func!\n");

   IRstate->next_line += 1;
}
