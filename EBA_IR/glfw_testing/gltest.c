#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

// tutorial from https://antongerdelan.net/opengl/hellotriangle.html

int main( void ) {
   // Start OpenGL context and OS window using the GLFW helper library.
   if ( !glfwInit() ) {
      fprintf( stderr, "ERROR: could not start GLFW3.\n" );
      return 1;
   }
   
   // Request an OpenGL 4.1, core, context from GLFW.
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
   glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
   
   // Create a window on the operating system, then tie the OpenGL context to it.
   GLFWwindow* window = glfwCreateWindow( 800, 600, "Hello Triangle", NULL, NULL );
   if ( !window ) {
      fprintf( stderr, "ERROR: Could not open window with GLFW3.\n" );
      glfwTerminate();
      return 1;
   }
   glfwMakeContextCurrent( window );
                                     
   // Start Glad, so we can call OpenGL functions.
   int version_glad = gladLoadGL( glfwGetProcAddress );
   if ( version_glad == 0 ) {
      fprintf( stderr, "ERROR: Failed to initialize OpenGL context.\n" );
      return 1;
   }
   printf( "Loaded OpenGL %i.%i\n", GLAD_VERSION_MAJOR( version_glad ), GLAD_VERSION_MINOR( version_glad ) );
   
   // Try to call some OpenGL functions, and print some more version info.
   printf( "Renderer: %s.\n", glGetString( GL_RENDERER ) );
   printf( "OpenGL version supported %s.\n", glGetString( GL_VERSION ) );
   
   /* OTHER STUFF GOES HERE NEXT */
   float tri_points[] =
   {
      +0.0f, +0.9f, +0.0f,
      +0.5f, -0.5f, +0.0f,
      -0.5f, -0.5f, +0.0f
   };

   // vertex buffer object
   GLuint vbo = 0;
   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), tri_points, GL_STATIC_DRAW);

   // vertex array object (array of vertex buffer objects (even just one!))
   GLuint vao = 0;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, vbo); // why not vao here?
   // not sure if that above line is needed either.
   // can remove it and things work fine!
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   
   int chars_read;
   FILE *vf = fopen("gltest.vert", "rb");
   fseek(vf, 0, SEEK_END);
   long int vfsize = ftell(vf);
   fseek(vf, 0, SEEK_SET);
   char *vertex_shader = malloc(vfsize+1);
   chars_read = fread(vertex_shader, 1, vfsize, vf);
   if(chars_read != vfsize)
   {
      printf("error in reading vertex shader!\n");
      exit(1);
   }
   fclose(vf);
   vertex_shader[vfsize] = '\0';

   void is_r_init(char *s)
   {
      int i;
      for(i = 0; s[i] != 0; i++)
      {
         printf("char %d: %c (%d)\n", i, s[i], s[i]);
         if (s[i] == '\r')
         {
            printf("\\r is indeed in the string\n");
         }
      }
   }

   FILE *ff = fopen("gltest.frag", "r");
   fseek(ff, 0, SEEK_END);
   long int ffsize = ftell(ff);
   fseek(ff, 0, SEEK_SET);
   char *fragment_shader = malloc(ffsize+1);
   chars_read = fread(fragment_shader, 1, ffsize, ff);
   if(chars_read != ffsize)
   {
      printf("error in reading fragment shader!\n");
      exit(1);
   }
   fclose(ff);
   fragment_shader[ffsize] = '\0';

   is_r_init(vertex_shader);
   printf("now fragment:\n");
   is_r_init(fragment_shader);

   
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

   while(!glfwWindowShouldClose(window))
   {
      glfwPollEvents();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader_program);
      glBindVertexArray(vao);

      glDrawArrays(GL_TRIANGLES, 0, 3);

      glfwSwapBuffers(window);
   }
     
   // Close OpenGL window, context, and any other GLFW resources.
   free(vertex_shader);
   free(fragment_shader);
   glfwTerminate();
   return 0;
}
