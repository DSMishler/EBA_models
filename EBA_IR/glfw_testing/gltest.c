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
   double tri_points[] =
   {
      +0.0f, +0.9f, +0.0f,
      +0.5f, -0.5f, +0.0f,
      -0.5f, -0.5f, +0.0f,
      -0.5f, -0.9f, +0.0f
      -0.5f, -0.8f, +0.0f,
      -0.9f, -0.7f, +0.0f,
   };

   // vertex buffer object
   GLuint vbo1 = 0;
   glGenBuffers(1, &vbo1);
   glBindBuffer(GL_ARRAY_BUFFER, vbo1);
   glBufferData(GL_ARRAY_BUFFER, 18*sizeof(double), tri_points, GL_STATIC_DRAW);

   // vertex array object (array of vertex buffer objects (even just one!))
   GLuint vao1 = 0;
   glGenVertexArrays(1, &vao1);
   glBindVertexArray(vao1);
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, vbo1); // why not vao1 here?
   // not sure if that above line is needed either.
   // can remove it and things work fine!
   glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, NULL);

   // second vertex buffer obj!
   GLuint vbo2 = 0;
   glGenBuffers(1, &vbo2);
   glBindBuffer(GL_ARRAY_BUFFER, vbo2);
   glBufferData(GL_ARRAY_BUFFER, 18*sizeof(double), tri_points, GL_STATIC_DRAW);

   // and vertex array
   GLuint vao2 = 0;
   glGenVertexArrays(1, &vao2);
   glBindVertexArray(vao2);
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, vbo2);
   glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, NULL);

   
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

   FILE *ff = fopen("gltest1.frag", "rb");
   fseek(ff, 0, SEEK_END);
   long int ffsize = ftell(ff);
   fseek(ff, 0, SEEK_SET);
   char *fragment_shader1 = malloc(ffsize+1);
   chars_read = fread(fragment_shader1, 1, ffsize, ff);
   if(chars_read != ffsize)
   {
      printf("error in reading fragment shader!\n");
      exit(1);
   }
   fclose(ff);
   fragment_shader1[ffsize] = '\0';

   ff = fopen("gltest2.frag", "rb");
   fseek(ff, 0, SEEK_END);
   ffsize = ftell(ff);
   fseek(ff, 0, SEEK_SET);
   char *fragment_shader2 = malloc(ffsize+1);
   chars_read = fread(fragment_shader2, 1, ffsize, ff);
   if(chars_read != ffsize)
   {
      printf("error in reading fragment shader!\n");
      exit(1);
   }
   fclose(ff);
   fragment_shader2[ffsize] = '\0';

   
   GLuint vs = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vs, 1, (const char**) &vertex_shader, NULL);
   glCompileShader(vs);

   GLuint fs1 = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fs1, 1, (const char**) &fragment_shader1, NULL);
   glCompileShader(fs1);

   GLuint fs2 = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fs2, 1, (const char**) &fragment_shader2, NULL);
   glCompileShader(fs2);

   GLuint shader_program1 = glCreateProgram();
   glAttachShader(shader_program1, fs1);
   glAttachShader(shader_program1, vs);
   glLinkProgram(shader_program1);

   GLuint shader_program2 = glCreateProgram();
   glAttachShader(shader_program2, fs2);
   glAttachShader(shader_program2, vs);
   glLinkProgram(shader_program2);
   glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

   while(!glfwWindowShouldClose(window))
   {
      glfwPollEvents();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader_program1);
      glBindVertexArray(vao1);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glUseProgram(shader_program2);
      glBindVertexArray(vao2);
      glDrawArrays(GL_TRIANGLES, 1, 3);
      // glDrawArrays(GL_LINES, 0, 6);
      // glDrawArrays(GL_POINTS, 0, 6);

      glfwSwapBuffers(window);
   }
     
   // Close OpenGL window, context, and any other GLFW resources.
   free(vertex_shader);
   free(fragment_shader1);
   glfwTerminate();
   return 0;
}
