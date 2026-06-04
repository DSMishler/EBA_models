#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdio.h>
#include <stdlib.h>

// personal test code to get GLFW running



// taken from the GLFW example code
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int width = 600;
int height = 500;
int channels = 4;

uint8_t *frame;
 

int main(void)
{
   printf("glfw tester\n");
   frame = malloc(1*channels*width*height);

   if (!glfwInit())
      exit(EXIT_FAILURE);


   GLFWwindow *window = glfwCreateWindow(width, height, "test OpenGL window", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }

   glfwSetKeyCallback(window, key_callback);

   glfwMakeContextCurrent(window);
   glfwSwapInterval(1);

   {
      int i;
      for(i = 0; i < channels*width*height; i++)
      {
         frame[i] = (char)((i * 256)/channels/width/height);
      }
   }


   const GLuint program = glCreateProgram();
   glLinkProgram(program);

   const GLint mvp_location = glGetUniformLocation(program, "MVP");

   GLuint tex;
   glGenTextures(1, &tex);

   while(!glfwWindowShouldClose(window))
   {
      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);

      glUseProgram(program);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      glBindTexture(GL_TEXTURE_2D, tex);
      glTexImage2D(
         GL_TEXTURE_2D,
         0,
         GL_RGBA4,
         width,
         height,
         0,
         GL_RGBA,
         GL_UNSIGNED_BYTE,
         frame);

      sleep(1);
      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   glfwDestroyWindow(window);


   glfwTerminate();
   free(frame);

   return 0;
}
