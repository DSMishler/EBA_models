#include <math.h>
#include <stdlib.h>
#include <stdio.h>
// #include <string.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <GLFW/glfw3.h>

int main(void)
{
   GLFWwindow *window;
   int width, height;


   if (!glfwInit())
   {
      fprintf(stderr, "failed init\n");
      exit(EXIT_FAILURE);
   }

   glfwWindowHint(GLFW_DEPTH_BITS, 16);
   glfwWindowHint(GLFW_TRANSPARENT_FRAMBUFFER, GLFW_TRUE);

   window = glfwCreateWindow( 300, 300, "Anything", NULL, NULL)
   if (!window)
   {
      fprintf(stderr, "failed window\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
   }



   glfwSetFramebufferSizeCallback(window, reshape);
   glfwSetKeyCallback(window, key);

   glfwMakeContextCurrent(window);
   // gladLoadGL(glfwGetProcAddress);
   glfwSwapInterval(1);

   glfwGetFramebufferSize(window, &width, &height);
   reshape(window, width, height)

   init();



   glfwTerminate();
   return 0;
}
