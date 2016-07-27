#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "luagl.h"


#if EMSCRIPTEN
#include <emscripten/emscripten.h>
#else
#define USE_GLEW 1
#endif

#include "SDL/SDL.h"

#if USE_GLEW
#include "GL/glew.h"
#else
#include "SDL/SDL_opengl.h"
#endif

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <assert.h>



static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

void draw()
{
    glClearColor( 0, 0, 0, 0 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
       0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    SDL_GL_SwapBuffers();
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

int CreateWindow(lua_State* L)
{
	 luaL_opengl(L);
    SDL_Surface *screen;
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    screen = SDL_SetVideoMode( 640, 480, 24, SDL_OPENGL );
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }
#if USE_GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
	    /* Problem: glewInit failed, something is seriously wrong. */
	    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	    exit(1);
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif


    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );

    glUseProgram(programID);


    return 0;
}
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode = "\n \
		attribute vec3 vertexPosition_modelspace;\n \
		void main(){gl_Position.xyz = vertexPosition_modelspace;gl_Position.w = 1.0;}";

	// Read the Fragment shader code from the file
	std::string FragmentShaderCode =
		"\n \
                void main(){gl_FragColor = vec4(0,1,0,0);}";

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

int time_to_next_frame()
{
	return 16;
}

int main (int argc, char *argv[])
{

  //load lua scripts
  FILE *file = fopen("lua/draw.lua", "rb");

  if (file==NULL) {fputs ("File error",stderr); return 1;}

  // obtain file size:
  fseek (file , 0 , SEEK_END);
  auto lSize = ftell (file);
  rewind (file);

  // allocate memory to contain the whole file:
  auto buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  auto result = fread (buffer,1,lSize,file);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  //close the file its now loaded into a memory buffer
  fclose (file);



//  glutInit(&argc, argv);
  int error;
  lua_State *L = luaL_newstate();   /* opens Lua */
  luaL_openlibs(L); /*open the lua libs*/


  //Register Create Window Function
  lua_register(L, "CreateWindow", CreateWindow);

  error = luaL_loadbuffer(L, buffer, strlen(buffer), "line") || lua_pcall(L, 0, 0, 0);
  free (buffer);

  if (error)
  {
    fprintf(stderr, "%s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
    return -1;
  }

#if EMSCRIPTEN
	  emscripten_set_main_loop(draw, 0, 1);
#else
  SDL_Event e;
  bool quit = false;
  while (!quit){
      while (SDL_PollEvent(&e)){
          if (e.type == SDL_QUIT){
              quit = true;
          }
          if (e.type == SDL_KEYDOWN){
              quit = true;
          }
          if (e.type == SDL_MOUSEBUTTONDOWN){
              quit = true;
          }
      }

        draw();
  }
#endif

SDL_Quit();


  free(buffer);
  lua_close(L);
  return 0;
}
