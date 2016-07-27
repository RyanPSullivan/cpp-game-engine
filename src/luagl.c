/*
The MIT License (MIT)

Copyright (c) 2015 Felipe Ferreira da Silva <felipefsdev@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "luagl.h"

#if EMSCRIPTEN

#else
#define USE_GLEW 1
#endif

#include "SDL/SDL.h"

#if USE_GLEW
#include "GL/glew.h"
#else
#include "SDL/SDL_opengl.h"
#endif



#define OPENGL_4_5 0
#define OPENGL_4_4 0
#define OPENGL_4_3 0
#define OPENGL_4_2 0
#define OPENGL_4_1 0
#define OPENGL_4_0 0
#define OPENGL_3_3 0
#define OPENGL_3_2 0
#define OPENGL_3_1 1
#define OPENGL_3_0 1
#define OPENGL_2_1 1
#define OPENGL_2_0 1

static double* checkarray_double(lua_State *L, int narg, int *len_out) {
    luaL_checktype(L, narg, LUA_TTABLE);

		int top = lua_gettop(L);
		int diff = top - narg;
    int len = lua_rawlen(L, narg);
    *len_out = len;
    double *buff = (double*)malloc(len*sizeof(double));

    for(int i = 0; i < len; i++) {
        lua_pushinteger(L, i+1);
	      lua_gettable(L, -2 - diff);
        if(lua_isnumber(L, -1)) {
            buff[i] = lua_tonumber(L, -1);
        } else {
            lua_pushfstring(L,
                strcat(
                    strcat(
                        "invalid entry #%d in array argument #%d (expected number, got ",
                        luaL_typename(L, -1)
                    ),
                    ")"
                ),
                i, narg
            );
            lua_error(L);
        }
        lua_pop(L, 1);
    }

    return buff;
}

static int lua_glDataToTable(lua_State *lua)
{
	size_t size = 0;
	GLenum type = luaL_checkinteger(lua, 1);
	const char *data = luaL_checklstring(lua, 2, &size);
	if (data) {
		GLsizei n_size = 1;
		if (type == GL_BYTE) {
			n_size = sizeof(char);
		} else if (type == GL_UNSIGNED_BYTE) {
			n_size = sizeof(unsigned char);
		} else if (type == GL_SHORT) {
			n_size = sizeof(short);
		} else if (type == GL_UNSIGNED_SHORT) {
			n_size = sizeof(unsigned short);
		} else if (type == GL_INT) {
			n_size = sizeof(int);
		} else if (type == GL_UNSIGNED_INT) {
			n_size = sizeof(unsigned int);
		} else if (type == GL_FLOAT) {
			n_size = sizeof(float);
		}
		lua_newtable(lua);
		int i = 0;
		while ((i * n_size) + n_size < size) {
			double value = 0;
			if (type == GL_BYTE) {
				char v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			} else if (type == GL_UNSIGNED_BYTE) {
				unsigned char v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			} else if (type == GL_SHORT) {
				short v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			} else if (type == GL_UNSIGNED_SHORT) {
				unsigned short v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			} else if (type == GL_INT) {
				int v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			} else if (type == GL_UNSIGNED_INT) {
				unsigned int v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			} else if (type == GL_FLOAT) {
				float v = 0;
				memcpy(&v, &data[i * n_size], n_size);
				value = v;
			}
			lua_pushinteger(lua, i);
			lua_pushnumber(lua, value);
			lua_settable(lua, -3);
			i = i + 1;
		}
	} else {
		lua_pushnil(lua);
	}
	return 1;
}

static int lua_glTableToData(lua_State *lua)
{
	char *data = NULL;
	GLsizei n_size = 1;
	GLenum type = luaL_checkinteger(lua, 1);
	if (type == GL_BYTE) {
		n_size = sizeof(char);
	} else if (type == GL_UNSIGNED_BYTE) {
		n_size = sizeof(unsigned char);
	} else if (type == GL_SHORT) {
		n_size = sizeof(short);
	} else if (type == GL_UNSIGNED_SHORT) {
		n_size = sizeof(unsigned short);
	} else if (type == GL_INT) {
		n_size = sizeof(int);
	} else if (type == GL_UNSIGNED_INT) {
		n_size = sizeof(unsigned int);
	} else if (type == GL_FLOAT) {
		n_size = sizeof(float);
	}
	luaL_checktype(lua, 2, LUA_TTABLE);
	GLsizei n = lua_rawlen(lua, 2);
	if ((lua_istable(lua, 2)) && (n > 0)) {
		data = (char*)malloc(n_size * n);
		int i = 0;
		for (i = 0; i < n; i++) {
			lua_rawgeti(lua, 2, i + 1);
			if (type == GL_BYTE) {
				char v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			} else if (type == GL_UNSIGNED_BYTE) {
				unsigned char v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			} else if (type == GL_SHORT) {
				short v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			} else if (type == GL_UNSIGNED_SHORT) {
				unsigned short v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			} else if (type == GL_INT) {
				int v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			} else if (type == GL_UNSIGNED_INT) {
				unsigned int v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			} else if (type == GL_FLOAT) {
				float v = lua_tonumber(lua, -1);
				memcpy(&data[i * n_size], &v, n_size);
			}
			lua_pop(lua, 1);
		}
	}
	if (data) {
		luaL_Buffer buffer;
		luaL_buffinit(lua, &buffer);
		luaL_addlstring(&buffer, (char*)data, n_size * n);
		luaL_pushresult(&buffer);
		free(data);
	} else {
		lua_pushnil(lua);
	}
	return 1;
}

static int lua_glEnable(lua_State *lua)
{
	glEnable(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glDisable(lua_State *lua)
{
	glDisable(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glIsEnabled(lua_State *lua)
{
	lua_pushboolean(lua, glIsEnabled(luaL_checkinteger(lua, 1)));
	return 1;
}

static int lua_glEnablei(lua_State *lua)
{
	glEnablei(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glDisablei(lua_State *lua)
{
	glDisablei(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glIsEnabledi(lua_State *lua)
{
	lua_pushboolean(lua, glIsEnabledi(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2)));
	return 0;
}

// BUFFER OBJECTS.

static int lua_glGenBuffer(lua_State *lua)
{
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	lua_pushinteger(lua, buffer);
	return 1;
}

static int lua_glGenBuffers(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *buffers =(GLuint*) malloc(n * sizeof(GLuint));
	glGenBuffers(n, buffers);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (const char*)buffers, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}

// static int lua_glCreateBuffer(lua_State *lua)
// {
// 	GLuint buffer = 0;
// 	glCreateBuffers(1, &buffer);
// 	lua_pushinteger(lua, buffer);
// 	return 1;
// }
//
// static int lua_glCreateBuffers(lua_State *lua)
// {
// 	GLsizei n = luaL_checkinteger(lua, 1);
// 	GLuint *buffers = (GLuint*)malloc(n * sizeof(GLuint));
// 	glCreateBuffers(n, buffers);
// 	luaL_Buffer buffer;
// 	luaL_buffinit(lua, &buffer);
// 	luaL_addlstring(&buffer, buffers, n * sizeof(GLuint));
// 	luaL_pushresult(&buffer);
// 	return 1;
// }

static int lua_glDeleteBuffer(lua_State *lua)
{
	GLuint buffer = luaL_checkinteger(lua, 1);
	glDeleteBuffers(1, &buffer);
	return 0;
}

static int lua_glDeleteBuffers(lua_State *lua)
{
	const GLuint *buffers = (GLuint*)luaL_checkstring(lua, 2);
	glDeleteBuffers(luaL_checkinteger(lua, 1), buffers);
	return 0;
}

// Create and Bind Buffer Objects.

static int lua_glBindBuffer(lua_State *lua)
{
	glBindBuffer(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glBindBufferRange(lua_State *lua)
{
	glBindBufferRange(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glBindBufferBase(lua_State *lua)
{
	glBindBufferBase(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

// Create/Modify Buffer Object Data.

static int lua_glBufferStorage(lua_State *lua)
{
	return 0;
}

static int lua_glNamedBufferStorage(lua_State *lua)
{
	return 0;
}

static int lua_glBufferData(lua_State *lua)
{
	int data_n = 0;

	int buffer = luaL_checkinteger(lua, 1);
	int usage =luaL_checkinteger(lua, 3);
	double* data = checkarray_double(lua, 2, &data_n);


	glBufferData(buffer, data_n, data, usage);

	return 0;
}

static int lua_glNamedBufferData(lua_State *lua)
{
	return 0;
}

static int lua_glBufferSubData(lua_State *lua)
{
	glBufferSubData(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkstring(lua, 4));
	return 0;
}

static int lua_glNamedBufferSubData(lua_State *lua)
{
	return 0;
}
//
// static int lua_glClearBufferSubData(lua_State *lua)
// {
// 	const char *data = NULL;
// 	if (lua_isstring(lua, 2)) {
// 		data = luaL_checkstring(lua, 7);
// 		glClearBufferSubData(luaL_checkinteger(lua, 1),
// 			luaL_checkinteger(lua, 2),
// 			luaL_checkinteger(lua, 3),
// 			luaL_checkinteger(lua, 4),
// 			luaL_checkinteger(lua, 5),
// 			luaL_checkinteger(lua, 6),
// 			data);
// 	} else {
// 		luaL_checktype(lua, 7, LUA_TNIL);
// 		glClearBufferSubData(luaL_checkinteger(lua, 1),
// 			luaL_checkinteger(lua, 2),
// 			luaL_checkinteger(lua, 3),
// 			luaL_checkinteger(lua, 4),
// 			luaL_checkinteger(lua, 5),
// 			luaL_checkinteger(lua, 6),
// 			NULL);
// 	}
// 	return 0;
// }

static int lua_glClearNamedBufferSubData(lua_State *lua)
{
	return 0;
}
//
// static int lua_glClearBufferData(lua_State *lua)
// {
// 	const char *data = NULL;
// 	if (lua_isstring(lua, 2)) {
// 		data = luaL_checkstring(lua, 5);
// 		glClearBufferData(luaL_checkinteger(lua, 1),
// 			luaL_checkinteger(lua, 2),
// 			luaL_checkinteger(lua, 3),
// 			luaL_checkinteger(lua, 4),
// 			data);
// 	} else {
// 		luaL_checktype(lua, 5, LUA_TNIL);
// 		glClearBufferData(luaL_checkinteger(lua, 1),
// 			luaL_checkinteger(lua, 2),
// 			luaL_checkinteger(lua, 3),
// 			luaL_checkinteger(lua, 4),
// 			NULL);
// 	}
// 	return 0;
// }

static int lua_glClearNamedBufferData(lua_State *lua)
{
	return 0;
}

// Map/Unmap Buffer Data.

static int lua_glMapBufferRange(lua_State *lua)
{
	void *data = glMapBufferRange(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	if (data) {
		luaL_Buffer buffer;
		luaL_buffinit(lua, &buffer);
		luaL_addlstring(&buffer, (char*)data, luaL_checkinteger(lua, 3));
		luaL_pushresult(&buffer);
	} else {
		lua_pushnil(lua);
	}
	return 1;
}

static int lua_glMapNamedBufferRange(lua_State *lua)
{
	return 0;
}

static int lua_glMapBuffer(lua_State *lua)
{
	return 0;
}

static int lua_glMapNamedBuffer(lua_State *lua)
{

	return 0;
}

static int lua_glFlushMappedBufferRange(lua_State *lua)
{

	return 0;
}

static int lua_glFlushMappedNamedBufferRange(lua_State *lua)
{

	return 0;
}

static int lua_glUnmapBuffer(lua_State *lua)
{

	return 0;
}

static int lua_glUnmapNamedBuffer(lua_State *lua)
{

	return 0;
}

// Invalidate Buffer Data.

static int lua_glInvalidadeBufferSubData(lua_State *lua)
{

	return 0;
}

static int lua_glInvalidadeBufferData(lua_State *lua)
{

	return 0;
}

// Buffer Object Queries.

static int lua_glIsBuffer(lua_State *lua)
{
	lua_pushboolean(lua, glIsBuffer(luaL_checkinteger(lua, 1)));
	return 1;
}

static int lua_glGetBufferSubData(lua_State *lua)
{

	return 0;
}

static int lua_glGetNamedBufferSubData(lua_State *lua)
{

	return 0;
}

static int lua_glGetBufferParameteriv(lua_State *lua)
{
	return 1;
}

static int lua_glGetNamedBufferParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glBufferPointerv(lua_State *lua)
{
	return 0;
}

// Copy Between Buffers.

static int lua_glCopyBufferSubData(lua_State *lua)
{
	return 0;
}

static int lua_glCopyNamedBufferSubData(lua_State *lua)
{
	return 0;
}

// SHADERS AND PROGRAMS.

// Shader Objects.

static int lua_glCreateShader(lua_State *lua)
{
	lua_pushinteger(lua, glCreateShader(luaL_checkinteger(lua, 1)));
	return 1;
}

static int lua_glShaderSource(lua_State *lua)
{
	const char *string = luaL_checkstring(lua, 2);
	glShaderSource(luaL_checkinteger(lua, 1),
		1,
		&string,
		NULL);
	return 0;
}

static int lua_glCompileShader(lua_State *lua)
{
	glCompileShader(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glReleaseShaderCompiler(lua_State *lua)
{
	glReleaseShaderCompiler();
	return 0;
}

static int lua_glDeleteShader(lua_State *lua)
{
	glDeleteShader(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glIsShader(lua_State *lua)
{
	lua_pushboolean(lua, glIsShader(luaL_checkinteger(lua, 1)));
	return 1;
}

static int lua_glShaderBinary(lua_State *lua)
{
	return 0;
}

// Program Objects.

static int lua_glCreateProgram(lua_State *lua)
{
	lua_pushinteger(lua, glCreateProgram());
	return 1;
}

static int lua_glAttachShader(lua_State *lua)
{
	glAttachShader(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glDetachShader(lua_State *lua)
{
	glDetachShader(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glLinkProgram(lua_State *lua)
{
	glLinkProgram(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glUseProgram(lua_State *lua)
{
	glUseProgram(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glDeleteProgram(lua_State *lua)
{
	glDeleteProgram(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glIsProgram(lua_State *lua)
{
	lua_pushboolean(lua, luaL_checkinteger(lua, 1));
	return 1;
}

// Program Interfaces.

// Program Pipeline Objects.

static int lua_glGenProgramPipeline(lua_State *lua)
{
	GLuint pipeline = 0;
	glGenProgramPipelines(1, &pipeline);
	lua_pushinteger(lua, pipeline);
	return 1;
}

static int lua_glGenProgramPipelines(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *pipelines = (GLuint*)malloc(n * sizeof(GLuint));
	glGenProgramPipelines(n, pipelines);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)pipelines, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glDeleteProgramPipeline(lua_State *lua)
{
	const GLuint pipeline = luaL_checkinteger(lua, 1);
	glDeleteProgramPipelines(1, &pipeline);
	return 0;
}

static int lua_glIsProgramPipeline(lua_State *lua)
{
	lua_pushboolean(lua, glIsProgramPipeline(luaL_checkinteger(lua, 1)));
	return 1;
}

// static int lua_glCreateProgramPipelines(lua_State *lua)
// {
// 	GLuint pipeline = 0;
// 	glCreateProgramPipelines(1, &pipeline);
// 	lua_pushinteger(lua, pipeline);
// 	return 1;
// }

static int lua_glActiveShaderProgram(lua_State *lua)
{
	glActiveShaderProgram(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// Program Binaries.

// Uniform Variables.

static int lua_glGetUniformLocation(lua_State *lua)
{
	GLuint program = luaL_checkinteger(lua, 1);
	const char *name = luaL_checkstring(lua, 2);
	GLuint location = glGetUniformLocation(program, name);
	lua_pushinteger(lua, location);
	return 1;
}

static int lua_glGetActiveUniformName(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformIndices(lua_State *lua)
{
	return 0;
}

static int lua_glGetActiveUniform(lua_State *lua)
{
	return 0;
}

static int lua_glGetActiveUniformsiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformBlockIndex(lua_State *lua)
{
	return 0;
}

static int lua_glGetActiveUniformBlockName(lua_State *lua)
{
	return 0;
}

static int lua_glGetActiveUniformBlockiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetActiveAtomicCounterBufferiv(lua_State *lua)
{
	return 0;
}

// Load Uniform Variables in Defaul Uniform Block.

static int lua_glUniform1i(lua_State *lua)
{
	glUniform1i(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glUniform2i(lua_State *lua)
{
	glUniform2i(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glUniform3i(lua_State *lua)
{
	glUniform3i(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glUniform4i(lua_State *lua)
{
	glUniform4i(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glUniform1f(lua_State *lua)
{
	glUniform1f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glUniform2f(lua_State *lua)
{
	glUniform2f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glUniform3f(lua_State *lua)
{
	glUniform3f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glUniform4f(lua_State *lua)
{
	glUniform4f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glUniform1d(lua_State *lua)
{
	glUniform1d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glUniform2d(lua_State *lua)
{
	glUniform2d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glUniform3d(lua_State *lua)
{
	glUniform3d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glUniform4d(lua_State *lua)
{
	glUniform4d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glUniform1ui(lua_State *lua)
{
	glUniform1ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glUniform2ui(lua_State *lua)
{
	glUniform2ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glUniform3ui(lua_State *lua)
{
	glUniform3ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glUniform4ui(lua_State *lua)
{
	glUniform4ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glUniform1iv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform2iv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform3iv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform4iv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform1fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform2fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform3fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform4fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform1dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform2dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform3dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform4dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform1uiv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform2uiv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform3uiv(lua_State *lua)
{
	return 0;
}

static int lua_glUniform4uiv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix2fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix3fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix4fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix2dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix3dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix4dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix2X3fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix3X2fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix2X4fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix4X2fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix3X4fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix4X3fv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix2X3dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix3X2dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix2X4dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix4X2dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix3X4dv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformMatrix4X3dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform1i(lua_State *lua)
{
	glUniform1i(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glProgramUniform2i(lua_State *lua)
{
	glUniform2i(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glProgramUniform3i(lua_State *lua)
{
	glUniform3i(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glProgramUniform4i(lua_State *lua)
{
	glUniform4i(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glProgramUniform1f(lua_State *lua)
{
	glUniform1f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glProgramUniform2f(lua_State *lua)
{
	glUniform2f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glProgramUniform3f(lua_State *lua)
{
	glUniform3f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glProgramUniform4f(lua_State *lua)
{
	glUniform4f(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glProgramUniform1d(lua_State *lua)
{
	glUniform1d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glProgramUniform2d(lua_State *lua)
{
	glUniform2d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glProgramUniform3d(lua_State *lua)
{
	glUniform3d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glProgramUniform4d(lua_State *lua)
{
	glUniform4d(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glProgramUniform1ui(lua_State *lua)
{
	glUniform1ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glProgramUniform2ui(lua_State *lua)
{
	glUniform2ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glProgramUniform3ui(lua_State *lua)
{
	glUniform3ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glProgramUniform4ui(lua_State *lua)
{
	glUniform4ui(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glProgramUniform1iv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform2iv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform3iv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform4iv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform1fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform2fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform3fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform4fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform1dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform2dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform3dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform4dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform1uiv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform2uiv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform3uiv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniform4uiv(lua_State *lua)
{
	return 0;
}

static int lua_gProgramlUniformMatrix2fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix3fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix4fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix2dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix3dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix4dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix2X3fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix3X2fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix2X4fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix4X2fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix3X4fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix4X3fv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix2X3dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix3X2dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix2X4dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix4X2dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix3X4dv(lua_State *lua)
{
	return 0;
}

static int lua_glProgramUniformMatrix4X3dv(lua_State *lua)
{
	return 0;
}

// Uniform Buffer Objects Bindings.

static int lua_glUniformBinding(lua_State *lua)
{
	return 0;
}

// Shader Buffer Variables.

static int lua_glShaderStorageBlockBindgins(lua_State *lua)
{
	return 0;
}

// Subroutine Uniform Variables.

static int lua_glGetSubroutineUniformLocation(lua_State *lua)
{
	return 0;
}

static int lua_glGetSubroutineIndex(lua_State *lua)
{
	return 0;
}

static int lua_glGetActiveSubroutineName(lua_State *lua)
{
	return 0;
}

static int lua_glActiveSubroutineUniformiv(lua_State *lua)
{
	return 0;
}

static int lua_glUniformSubroutinesuiv(lua_State *lua)
{
	return 0;
}

// Shader Memory Access.

static int lua_glMemoryBarrier(lua_State *lua)
{
	return 0;
}

static int lua_glMemoryBarrierByRegion(lua_State *lua)
{
	return 0;
}

// Shader and Program Queries.

static int lua_glGetShaderiv(lua_State *lua)
{
	GLint params = 0;
	glGetShaderiv(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		&params);
	lua_pushinteger(lua, params);
	return 1;
}

static int lua_glGetProgramiv(lua_State *lua)
{
	GLint params = 0;
	glGetShaderiv(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		&params);
	lua_pushinteger(lua, params);
	return 1;
}

static int lua_glGetProgramPipelineiv(lua_State *lua)
{
	return 0;
}

static int lua_glAttachedShaders(lua_State *lua)
{
	return 0;
}

static int lua_glGetShaderInfoLog(lua_State *lua)
{
	int len = 0;
	glGetShaderiv(luaL_checkinteger(lua, 1),
		GL_INFO_LOG_LENGTH,
		&len);
	if (len > 0) {
		char *info = (char*)malloc(len);
		glGetShaderInfoLog(luaL_checkinteger(lua, 1),
			len,
			NULL,
			info);
		luaL_Buffer buffer;
		luaL_buffinit(lua, &buffer);
		luaL_addlstring(&buffer, (char*)info, len);
		luaL_pushresult(&buffer);
	} else {
		lua_pushnil(lua);
	}
	return 1;
}

static int lua_glProgramInfoLog(lua_State *lua)
{
	return 0;
}

static int lua_glProgramPipelineInfoLog(lua_State *lua)
{
	return 0;
}

static int lua_glGetShaderSource(lua_State *lua)
{
	return 0;
}

static int lua_glGetShaderPrecisionFormat(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformi(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformf(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformd(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformui(lua_State *lua)
{
	return 0;
}

static int lua_glGetnUniformi(lua_State *lua)
{
	return 0;
}

static int lua_glGetnUniformf(lua_State *lua)
{
	return 0;
}

static int lua_glGetnUniformd(lua_State *lua)
{
	return 0;
}

static int lua_glGetnUniformui(lua_State *lua)
{
	return 0;
}

static int lua_glGetUniformSubroutineuiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetProgramStageiv(lua_State *lua)
{
	return 0;
}

// TEXTURES AND SAMPLERS.

static int lua_glActiveTexture(lua_State *lua)
{
	glActiveTexture(luaL_checkinteger(lua, 1));
	return 0;
}

// Texture Objects.

static int lua_glGenTexture(lua_State *lua)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	lua_pushinteger(lua, texture);
	return 1;
}

static int lua_glGenTextures(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *textures = (GLuint*)malloc(n * sizeof(GLuint));
	glGenTextures(n, textures);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)textures, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glBindTexture(lua_State *lua)
{
	glBindTexture(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glBindTextures(lua_State *lua)
{
	return 0;
}

// static int lua_glBindTextureUnit(lua_State *lua)
// {
// 	glBindTextureUnit(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 1));
// 	return 0;
// }

static int lua_glCreateTextures(lua_State *lua)
{
	return 0;
}

static int lua_glDeleteTextures(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	const char *textures = luaL_checkstring(lua, 2);
	glDeleteTextures(n, (const GLuint *)textures);
	return 0;
}

static int lua_glIsTexture(lua_State *lua)
{
	lua_pushboolean(lua, glIsTexture(luaL_checkinteger(lua, 1)));
	return 1;
}

// Sampler Objects.

static int lua_glGenSampler(lua_State *lua)
{
	GLuint sampler = 0;
	glGenSamplers(1, &sampler);
	lua_pushinteger(lua, sampler);
	return 1;
}

static int lua_glGenSamplers(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *samplers = (GLuint*)malloc(n * sizeof(GLuint));
	glGenSamplers(n, samplers);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)samplers, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}
//
// static int (lua_State *lua)
// {
// 	GLuint sampler = 0;
// 	glCreateSamplers(1, &sampler);
// 	lua_pushinteger(lua, sampler);
// 	return 1;
// }
//
// static int lua_glCreateSamplers(lua_State *lua)
// {
// 	GLsizei n = luaL_checkinteger(lua, 1);
// 	GLuint *samplers = (GLuint*)malloc(n * sizeof(GLuint));
// 	glCreateSamplers(n, samplers);
// 	luaL_Buffer buffer;
// 	luaL_buffinit(lua, &buffer);
// 	luaL_addlstring(&buffer, (char*)samplers, n * sizeof(GLuint));
// 	luaL_pushresult(&buffer);
// 	return 1;
// }

static int lua_glBindSampler(lua_State *lua)
{
	glBindSampler(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glBindSamplers(lua_State *lua)
{
	return 0;
}

static int lua_glSamplerParameteri(lua_State *lua)
{
	return 0;
}

static int lua_glSamplerParameterf(lua_State *lua)
{
	return 0;
}

static int lua_glSamplerParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glSamplerParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glSamplerParameterIiv(lua_State *lua)
{
	return 0;
}

static int lua_glSamplerParameterIuiv(lua_State *lua)
{
	return 0;
}

static int lua_glDeleteSamplers(lua_State *lua)
{
	return 0;
}

static int lua_glIsSampler(lua_State *lua)
{
	lua_pushboolean(lua, glIsSampler(luaL_checkinteger(lua, 1)));
	return 1;
}

// Sampler Queries.

static int lua_glGetSamplerParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetSamplerParamenterfv(lua_State *lua)
{
	return 0;
}

static int lua_glGetSamplerParameterIiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetSamplerParameterIfv(lua_State *lua)
{
	return 0;
}

// Pixel Storate Modes.

static int lua_glPixelStorei(lua_State *lua)
{
	glPixelStorei(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glPixelStoref(lua_State *lua)
{
	glPixelStoref(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

// Texture Image Spec.

static int lua_glTexImage3D(lua_State *lua)
{
	if (lua_type(lua, 10) == LUA_TNIL) {
		glTexImage3D(luaL_checkinteger(lua, 1),
			luaL_checkinteger(lua, 2),
			luaL_checkinteger(lua, 3),
			luaL_checkinteger(lua, 4),
			luaL_checkinteger(lua, 5),
			luaL_checkinteger(lua, 6),
			luaL_checkinteger(lua, 7),
			luaL_checkinteger(lua, 8),
			luaL_checkinteger(lua, 9),
			NULL);
	} else {
		glTexImage3D(luaL_checkinteger(lua, 1),
			luaL_checkinteger(lua, 2),
			luaL_checkinteger(lua, 3),
			luaL_checkinteger(lua, 4),
			luaL_checkinteger(lua, 5),
			luaL_checkinteger(lua, 6),
			luaL_checkinteger(lua, 7),
			luaL_checkinteger(lua, 8),
			luaL_checkinteger(lua, 9),
			luaL_checkstring(lua, 10));
	}
	return 0;
}

static int lua_glTexImage2D(lua_State *lua)
{
	if (lua_type(lua, 9) == LUA_TNIL) {
		glTexImage2D(luaL_checkinteger(lua, 1),
			luaL_checkinteger(lua, 2),
			luaL_checkinteger(lua, 3),
			luaL_checkinteger(lua, 4),
			luaL_checkinteger(lua, 5),
			luaL_checkinteger(lua, 6),
			luaL_checkinteger(lua, 7),
			luaL_checkinteger(lua, 8),
			NULL);
	} else {
		glTexImage2D(luaL_checkinteger(lua, 1),
			luaL_checkinteger(lua, 2),
			luaL_checkinteger(lua, 3),
			luaL_checkinteger(lua, 4),
			luaL_checkinteger(lua, 5),
			luaL_checkinteger(lua, 6),
			luaL_checkinteger(lua, 7),
			luaL_checkinteger(lua, 8),
			luaL_checkstring(lua, 9));
	}
	return 0;
}

static int lua_glTexImage1D(lua_State *lua)
{
	if (lua_type(lua, 8) == LUA_TNIL) {
		glTexImage1D(luaL_checkinteger(lua, 1),
			luaL_checkinteger(lua, 2),
			luaL_checkinteger(lua, 3),
			luaL_checkinteger(lua, 4),
			luaL_checkinteger(lua, 5),
			luaL_checkinteger(lua, 6),
			luaL_checkinteger(lua, 7),
			NULL);
	} else {
		glTexImage1D(luaL_checkinteger(lua, 1),
			luaL_checkinteger(lua, 2),
			luaL_checkinteger(lua, 3),
			luaL_checkinteger(lua, 4),
			luaL_checkinteger(lua, 5),
			luaL_checkinteger(lua, 6),
			luaL_checkinteger(lua, 7),
			luaL_checkstring(lua, 8));
	}
	return 0;
}

// Alternate Texture Image Spec.

static int lua_glCopyTexImage2D(lua_State *lua)
{
	glCopyTexImage2D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkinteger(lua, 7),
		luaL_checkinteger(lua, 8));
	return 0;
}

static int lua_glCopyTexImage1D(lua_State *lua)
{
	glCopyTexImage1D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkinteger(lua, 7));
	return 0;
}

static int lua_glTexSubImage3D(lua_State *lua)
{
	glTexSubImage3D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkinteger(lua, 7),
		luaL_checkinteger(lua, 8),
		luaL_checkinteger(lua, 9),
		luaL_checkinteger(lua, 10),
		luaL_checkstring(lua, 11));
	return 0;
}

static int lua_glTexSubImage2D(lua_State *lua)
{
	glTexSubImage2D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkinteger(lua, 7),
		luaL_checkinteger(lua, 8),
		luaL_checkstring(lua, 9));
	return 0;
}

static int lua_glTexSubImage1D(lua_State *lua)
{
	glTexSubImage1D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkstring(lua, 7));
	return 0;
}

static int lua_glCopyTexSubImage3D(lua_State *lua)
{
	glCopyTexSubImage3D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkinteger(lua, 7),
		luaL_checkinteger(lua, 8),
		luaL_checkinteger(lua, 9));
	return 0;
}

static int lua_glCopyTexSubImage2D(lua_State *lua)
{
	glCopyTexSubImage2D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6),
		luaL_checkinteger(lua, 7),
		luaL_checkinteger(lua, 8));
	return 0;
}

static int lua_glCopyTexSubImage1D(lua_State *lua)
{
	glCopyTexSubImage1D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6));
	return 0;
}

static int lua_glTextureSubImage3D(lua_State *lua)
{
	return 0;
}

static int lua_glTextureSubImage2D(lua_State *lua)
{
	return 0;
}

static int lua_glTextureSubImage1D(lua_State *lua)
{
	return 0;
}

static int lua_glCopyTextureSubImage3D(lua_State *lua)
{
	return 0;
}

static int lua_glCopyTextureSubImage2D(lua_State *lua)
{
	return 0;
}

static int lua_glCopyTextureSubImage1D(lua_State *lua)
{
	return 0;
}

// Compressed Texture Images.

static int lua_glCompressedTexImage3D(lua_State *lua)
{
	return 0;
}

static int lua_glCompressedTexImage2D(lua_State *lua)
{
	return 0;
}

static int lua_glCompressedTexImage1D(lua_State *lua)
{
	return 0;
}

static int lua_glCompressedTexSubImage3D(lua_State *lua)
{
	return 0;
}

static int lua_glCompressedTexSubImage2D(lua_State *lua)
{
	return 0;
}

static int lua_glCompressedTexSubImage1D(lua_State *lua)
{
	return 0;
}

// Multisample Textures.

static int lua_glTexImage3DMultisample(lua_State *lua)
{
	return 0;
}

static int lua_glTexImage2DMultisample(lua_State *lua)
{
	return 0;
}

// Buffer Textures.

// static int lua_glTexBufferRange(lua_State *lua)
// {
// 	glTexBufferRange(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4),
// 		luaL_checkinteger(lua, 5));
// 	return 0;
// }

static int lua_glTextureBufferRange(lua_State *lua)
{
	return 0;
}

static int lua_glTexBuffer(lua_State *lua)
{
	glTexBuffer(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glTextureBuffer(lua_State *lua)
{
	return 0;
}

// Texture Parameters.

static int lua_glTexParameteri(lua_State *lua)
{
	return 0;
}

static int lua_glTexParameterf(lua_State *lua)
{
	return 0;
}

static int lua_glTexParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glTexParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glTexParameterIiv(lua_State *lua)
{
	return 0;
}

static int lua_glTexParamenterIuiv(lua_State *lua)
{
	return 0;
}

static int lua_glTextureParamenteri(lua_State *lua)
{
	return 0;
}

static int lua_glTextureParamenterf(lua_State *lua)
{
	return 0;
}

static int lua_glTextureParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glTextureParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glTextureParameterIiv(lua_State *lua)
{
	return 0;
}

static int lua_glTextureParameterIuiv(lua_State *lua)
{
	return 0;
}

// Texture Queries.

static int lua_glGetTexParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTexParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTexParameterIiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTexParameterIuiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTextureParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTextureParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTextureParameterIiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTextureParameterIuiv(lua_State *lua)
{
	return 0;
}

static int lua_glTexLevelParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glTexLevelParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glTextureLevelParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glTextureLevelParameterfv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTexImage(lua_State *lua)
{
	GLvoid *img = NULL;
	GLsizei size = 0;
	GLenum target = luaL_checkinteger(lua, 1);
	GLint level = luaL_checkinteger(lua, 2);
	if (target == GL_TEXTURE_1D) {
		GLint width = 0;
		GLint format = 0;
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_WIDTH,
			&width);
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_INTERNAL_FORMAT,
			&format);
	} else if (target == GL_TEXTURE_2D) {
		GLint width = 0;
		GLint height = 0;
		GLint format = 0;
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_WIDTH,
			&width);
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_HEIGHT,
			&height);
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_INTERNAL_FORMAT,
			&format);
	} else if (target == GL_TEXTURE_3D) {
		GLint width = 0;
		GLint height = 0;
		GLint depth = 0;
		GLint format = 0;
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_WIDTH,
			&width);
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_HEIGHT,
			&height);
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_DEPTH,
			&depth);
		glGetTexLevelParameteriv(target,
			level,
			GL_TEXTURE_INTERNAL_FORMAT,
			&format);
	}
	if (size > 0) {
		img = (GLvoid*)malloc(size);
	}
	glGetTexImage(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		img);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)img, size);
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glGetTextureImage(lua_State *lua)
{
	return 0;
}

static int lua_glGetnTexImage(lua_State *lua)
{
	return 0;
}

static int lua_glGetTextureSubImage(lua_State *lua)
{
	return 0;
}

static int lua_glGetCompressedTexImage(lua_State *lua)
{
	return 0;
}

static int lua_glGetCompressedTextureImage(lua_State *lua)
{
	return 0;
}

static int lua_glGetnCompressedTexImage(lua_State *lua)
{
	return 0;
}

static int lua_glGetCompressedTextureSubImage(lua_State *lua)
{
	return 0;
}

// Cube Map Texture Select.

// Manual Mipmap Generation.

static int lua_glGenerateMipmap(lua_State *lua)
{
	glGenerateMipmap(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glGenerateTextureMipmap(lua_State *lua)
{
	return 0;
}

// Texture Views.

// static int lua_glTextureView(lua_State *lua)
// {
// 	glTextureView(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4),
// 		luaL_checkinteger(lua, 5),
// 		luaL_checkinteger(lua, 6),
// 		luaL_checkinteger(lua, 7),
// 		luaL_checkinteger(lua, 8));
// 	return 0;
// }

// Immutable-Format Tex. Images.
//
// static int lua_glTexStorate1D(lua_State *lua)
// {
// 	glTexStorage1D(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4));
// 	return 0;
// }
//
// static int lua_glTexStorage2D(lua_State *lua)
// {
// 	glTexStorage2D(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4),
// 		luaL_checkinteger(lua, 5));
// 	return 0;
// }
//
// static int lua_glTexStorage3D(lua_State *lua)
// {
// 	glTexStorage3D(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4),
// 		luaL_checkinteger(lua, 5),
// 		luaL_checkinteger(lua, 6));
// 	return 0;
// }

static int lua_glTextureStorage1D(lua_State *lua)
{
	return 0;
}

static int lua_glTextureStorage2D(lua_State *lua)
{
	return 0;
}

static int lua_glTextureStorage3D(lua_State *lua)
{
	return 0;
}

static int lua_glTexStorage2DMultisample(lua_State *lua)
{
	return 0;
}

static int lua_glTexStorage3DMultisample(lua_State *lua)
{
	return 0;
}

static int lua_glTextureStorage2DMultisample(lua_State *lua)
{
	return 0;
}

static int lua_glTextureStorage3DMultisample(lua_State *lua)
{
	return 0;
}

// Invalidade Texture Image Data.

static int lua_glInvalidadeTexSubImage(lua_State *lua)
{
	return 0;
}

static int lua_glInvalidadeTexImage(lua_State *lua)
{
	return 0;
}

// Clear Texture Image Data.

static int lua_glClearTexSubImage(lua_State *lua)
{
	return 0;
}

static int lua_glClearTexImage(lua_State *lua)
{
	return 0;
}

// Texture Image Loads/Store.

// static int lua_glBindImageTexture(lua_State *lua)
// {
// 	glBindImageTexture(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4),
// 		luaL_checkinteger(lua, 5),
// 		luaL_checkinteger(lua, 6),
// 		luaL_checkinteger(lua, 7));
// 	return 0;
// }
//
// static int lua_glBindImageTextures(lua_State *lua)
// {
// 	return 0;
// }

// FRAMEBUFFER OBJECTS.

// Binding and Managing.

static int lua_glBindFramebuffer(lua_State *lua)
{
	glBindFramebuffer(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// static int lua_glCreateFramebuffer(lua_State *lua)
// {
// 	GLuint framebuffer = 0;
// 	glCreateFramebuffers(1, &framebuffer);
// 	lua_pushinteger(lua, framebuffer);
// 	return 1;
// }
//
// static int lua_glCreateFramebuffers(lua_State *lua)
// {
// 	return 0;
// }

static int lua_glGenFramebuffer(lua_State *lua)
{
	GLuint framebuffer = 0;
	glGenFramebuffers(1, &framebuffer);
	lua_pushinteger(lua, framebuffer);
	return 1;
}

static int lua_glGenFramebuffers(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *framebuffers = (GLuint*)malloc(n * sizeof(GLuint));
	glGenFramebuffers(n, framebuffers);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)framebuffers, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glDeleteFramebuffer(lua_State *lua)
{
	GLuint framebuffer = luaL_checkinteger(lua, 1);
	glDeleteFramebuffers(1, &framebuffer);
	return 0;
}

static int lua_glDeleteFramebuffers(lua_State *lua)
{
	GLuint n = luaL_checkinteger(lua, 1);
	const char *buffers = luaL_checkstring(lua, 2);
	glDeleteFramebuffers(n, (const GLuint*)buffers);
	return 0;
}

static int lua_glIsFramebuffer(lua_State *lua)
{
	lua_pushboolean(lua, glIsFramebuffer(luaL_checkinteger(lua, 1)));
	return 1;
}

// Framebuffer Object Parameters.

static int lua_glFramebufferParameteri(lua_State *lua)
{
	return 0;
}

static int lua_glNamedFramebufferParameteri(lua_State *lua)
{
	return 0;
}

// Framebuffer Object Queries.

static int lua_glGetFramebufferParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetNamedFramebufferParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetFramebufferAttachmentParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetNamedFramebufferAttachmentParameteriv(lua_State *lua)
{
	return 0;
}

// Renderbuffer Objects.

static int lua_glBindRenderbuffer(lua_State *lua)
{
	glBindRenderbuffer(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// static int lua_glCreateRenderbuffer(lua_State *lua)
// {
// 	GLuint renderbuffer = 0;
// 	glCreateRenderbuffers(1, &renderbuffer);
// 	lua_pushinteger(lua, renderbuffer);
// 	return 1;
// }

// static int lua_glCreateRenderbuffers(lua_State *lua)
// {
// 	return 0;
// }

static int lua_glGenRenderbuffer(lua_State *lua)
{
	GLuint renderbuffer = 0;
	glGenRenderbuffers(1, &renderbuffer);
	lua_pushinteger(lua, renderbuffer);
	return 1;
}

static int lua_glGenRenderbuffers(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *renderbuffers = (GLuint*)malloc(n * sizeof(GLuint));
	glGenRenderbuffers(n, renderbuffers);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)renderbuffers, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glDeleteRenderbuffer(lua_State *lua)
{
	return 0;
}

static int lua_glDeleteRenderbuffers(lua_State *lua)
{
	return 0;
}

static int lua_glIsRenderbuffer(lua_State *lua)
{
	lua_pushboolean(lua, glIsRenderbuffer(luaL_checkinteger(lua, 1)));
	return 1;
}

static int lua_glRenderbufferStorageMultisample(lua_State *lua)
{
	glRenderbufferStorageMultisample(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glNamedRenderbufferStorageMultisample(lua_State *lua)
{
	return 0;
}

static int lua_glRenderbufferStorage(lua_State *lua)
{
	glRenderbufferStorage(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glNamedRenderbufferStorage(lua_State *lua)
{
	return 0;
}

// Renderbuffer Object Queries.

static int lua_glGetRenderbufferParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glGetNamedRenderbufferParameteriv(lua_State *lua)
{
	return 0;
}

// Attaching Renderbuffer Images.

static int lua_glFramebufferRenderbuffer(lua_State *lua)
{
	glFramebufferRenderbuffer(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glNamedFramebufferRenderbuffer(lua_State *lua)
{
	return 0;
}

// Attaching Texture Images.

static int lua_glFramebufferTexture(lua_State *lua)
{
	glFramebufferTexture(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glNamedFramebufferTexture(lua_State *lua)
{
	return 0;
}

static int lua_glFramebufferTexture1D(lua_State *lua)
{
	glFramebufferTexture1D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glFramebufferTexture2D(lua_State *lua)
{
	glFramebufferTexture2D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glFramebufferTexture3D(lua_State *lua)
{
	glFramebufferTexture3D(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5),
		luaL_checkinteger(lua, 6));
	return 0;
}

static int lua_glFramebufferTextureLayer(lua_State *lua)
{
	return 0;
}

static int lua_glNamedFramebufferTextureLayer(lua_State *lua)
{
	return 0;
}

// Feedback Loops.

static int lua_glTextureBarrier(lua_State *lua)
{
	return 0;
}

// Framebuffer Completeness.

static int lua_glCheckFramebufferStatus(lua_State *lua)
{
	lua_pushinteger(lua, glCheckFramebufferStatus(luaL_checkinteger(lua, 1)));
	return 1;
}

static int lua_glCheckNamedFramebufferStatus(lua_State *lua)
{
	return 0;
}

// VERTICES.

// Separate Patches.

static int lua_glPatchParameteri(lua_State *lua)
{
	return 0;
}

// Current Vertex Attribute Values.

static int lua_glVertexAttrib1s(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib2s(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib3s(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4s(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib1f(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib2f(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib3f(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4f(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib1d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib2d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib3d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib1sv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib2sv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib3sv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib1fv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib2fv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib3fv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib1dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib2dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib3dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4bv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4sv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4iv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4fv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4ubv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4usv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nub(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nb(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nbv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nsv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Niv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nubv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nusv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttrib4Nuiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI1i(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI2i(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI3i(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4i(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI1ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI2ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI3ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI1uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI2uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI3uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4bv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4sv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4ubv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribI4usv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL1d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL2d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL3d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL4d(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL1dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL2dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL3dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribL4dv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP1ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP2ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP3ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP4ui(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP1uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP2uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP3uiv(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribP4uiv(lua_State *lua)
{
	return 0;
}

// VERTEX ARRAYS.

// Vertex Array Objects.

static int lua_glGenVertexArray(lua_State *lua)
{
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	lua_pushinteger(lua, vao);
	return 1;
}

static int lua_glGenVertexArrays(lua_State *lua)
{
	GLsizei n = luaL_checkinteger(lua, 1);
	GLuint *vaos = (GLuint*)malloc(n * sizeof(GLuint));
	glGenVertexArrays(n, vaos);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)vaos, n * sizeof(GLuint));
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glDeleteVertexArrays(lua_State *lua)
{
	return 0;
}

static int lua_glBindVertexArray(lua_State *lua)
{
	glBindVertexArray(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glCreateVertexArrays(lua_State *lua)
{
	return 0;
}

static int lua_glIsVertexArray(lua_State *lua)
{
	glIsVertexArray(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glVertexArrayElementBuffer(lua_State *lua)
{
	return 0;
}

// Generic Vertex Attribute Arrays.

static int lua_glVertexAttribFormat(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribIFormat(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribLFormat(lua_State *lua)
{
	return 0;
}

static int lua_glVertexArrayAttribFormat(lua_State *lua)
{
	return 0;
}

static int lua_glVertexArrayAttribIFormat(lua_State *lua)
{
	return 0;
}

static int lua_glVertexArrayAttribLFormat(lua_State *lua)
{
	return 0;
}

// static int lua_glBindVertexBuffer(lua_State *lua)
// {
// 	glBindVertexBuffer(luaL_checkinteger(lua, 1),
// 		luaL_checkinteger(lua, 2),
// 		luaL_checkinteger(lua, 3),
// 		luaL_checkinteger(lua, 4));
// 	return 0;
// }

static int lua_glVertexArrayVertexBuffer(lua_State *lua)
{
	return 0;
}

static int lua_glBindVertexBuffers(lua_State *lua)
{
	return 0;
}

static int lua_glVertexArrayVertexBuffers(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribBinding(lua_State *lua)
{
	return 0;
}

static int lua_glVertexArrayAttribBinding(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribPointer(lua_State *lua)
{
	glVertexAttribPointer(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5) * sizeof(GLfloat),
		NULL);
	return 0;
}

static int lua_glVertexAttribLPointer(lua_State *lua)
{
	return 0;
}

static int lua_glEnableVertexAttribArray(lua_State *lua)
{
	glEnableVertexAttribArray(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glEnableVertexArrayAttrib(lua_State *lua)
{
	return 0;
}

static int lua_glDisableVertexAttribArray(lua_State *lua)
{
	return 0;
}

static int lua_glDisableVertexArrayAttrib(lua_State *lua)
{
	return 0;
}

// Vertex Attribute Divisors.

static int lua_glVertexBindingDivisor(lua_State *lua)
{
	return 0;
}

static int lua_glVertexArrayBindingDivisor(lua_State *lua)
{
	return 0;
}

static int lua_glVertexAttribDivisor(lua_State *lua)
{
	return 0;
}

// Primitive Restart.

static int lua_glPrimitiveRestartIndex(lua_State *lua)
{
	glPrimitiveRestartIndex(luaL_checkinteger(lua, 1));
	return 0;
}

// Drawing Commands.

static int lua_glDrawArrays(lua_State *lua)
{
	glDrawArrays(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glDrawArraysInstancedBasedInstance(lua_State *lua)
{
	return 0;
}

static int lua_glDrawArraysInstanced(lua_State *lua)
{
	return 0;
}

static int lua_glDrawArraysIndirect(lua_State *lua)
{
	return 0;
}

static int lua_glMultiDrawArrays(lua_State *lua)
{
	return 0;
}

static int lua_glMultiDrawArraysIndirect(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElements(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElementsInstancedBaseInstance(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElementInstanced(lua_State *lua)
{
	return 0;
}

static int lua_glMultiDrawElements(lua_State *lua)
{
	return 0;
}

static int lua_glDrawRangeElements(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElementsBaseVertex(lua_State *lua)
{
	return 0;
}

static int lua_glDrawRangeElementsBaseVertex(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElementsInstancedBaseVertex(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElementsInstancedBaseVertexBaseInstance(lua_State *lua)
{
	return 0;
}

static int lua_glDrawElementsIndirect(lua_State *lua)
{
	return 0;
}

static int lua_glMultiDrawElementsIndirect(lua_State *lua)
{
	return 0;
}

static int lua_glMultiDrawElementsBaseVertex(lua_State *lua)
{
	return 0;
}

// Vertex Array Queries.

static int lua_glGetVertexArrayiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexArrayIndexdiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexArrayIndexd64iv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribdv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribfv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribIiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribIuiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribLdv(lua_State *lua)
{
	return 0;
}

static int lua_glGetVertexAttribPointerv(lua_State *lua)
{
	return 0;
}

// Conditional Rendering.

static int lua_glBeginConditionalRender(lua_State *lua)
{
	glBeginConditionalRender(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glEndConditionalRender(lua_State *lua)
{
	return 0;
}

// VERTEX ATTRIBUTES

static int lua_glBindAttribLocation(lua_State *lua)
{
	glBindAttribLocation(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkstring(lua, 3));
	return 0;
}

static int lua_glGetActiveAttrib(lua_State *lua)
{
	return 0;
}

static int lua_glGetAttribLocation(lua_State *lua)
{
	lua_pushinteger(lua, glGetAttribLocation(luaL_checkinteger(lua, 1),
		luaL_checkstring(lua, 2)));
	return 1;
}

// Transform Feedback Variables.

static int lua_glTransformFeedbackVaryings(lua_State *lua)
{
	return 0;
}

static int lua_glGetTransformFeedbackVarying(lua_State *lua)
{
	return 0;
}

// Shader Execution.

static int lua_glValidateProgram(lua_State *lua)
{
	glValidateProgram(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glValidadteProgramPipeline(lua_State *lua)
{
	glValidateProgramPipeline(luaL_checkinteger(lua, 1));
	return 0;
}

// Tesselation Prim. Generation.

static int lua_glPatchParameterfv(lua_State *lua)
{
	return 0;
}

// VERTEX POST-PROCESSING.

// Transform Feedback.

static int lua_glGenTransformFeedbacks(lua_State *lua)
{
	return 0;
}

static int lua_glDeleteTransformFeedbacks(lua_State *lua)
{
	return 0;
}

static int lua_glBindTransformFeedback(lua_State *lua)
{
	return 0;
}

static int lua_glCreateTransformFeedbacks(lua_State *lua)
{
	return 0;
}

static int lua_glBeginTransformFeedback(lua_State *lua)
{
	return 0;
}

static int lua_glEndTransformFeedback(lua_State *lua)
{
	return 0;
}

static int lua_glPauseTransformFeedback(lua_State *lua)
{
	return 0;
}

static int lua_glResumeTransformFeedback(lua_State *lua)
{
	return 0;
}

static int lua_glTransformFeedbackBufferRange(lua_State *lua)
{
	return 0;
}

static int lua_glTransformFeedbackBufferBase(lua_State *lua)
{
	return 0;
}

// Transform Feedback Drawing.

static int lua_glDrawTransformFeedback(lua_State *lua)
{
	glDrawTransformFeedback(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glDrawTransformFeedbackInstanced(lua_State *lua)
{
	return 0;
}

static int lua_glDrawTransformFeedbackStream(lua_State *lua)
{
	return 0;
}

static int lua_glDrawTransformFeedbackStreamInstanced(lua_State *lua)
{
	return 0;
}

// Flatshading.

static int lua_glProvokingVertex(lua_State *lua)
{
	glProvokingVertex(luaL_checkinteger(lua, 1));
	return 0;
}

// Primitive Clipping.

static int lua_glClipControl(lua_State *lua)
{
	return 0;
}

// Controlling Viewport

static int lua_glDepthRangeArrayv(lua_State *lua)
{
	const char *v = luaL_checkstring(lua, 3);
	glDepthRangeArrayv(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		(const GLclampd *)v);
	return 0;
}

static int lua_glDepthRangeIndexed(lua_State *lua)
{
	glDepthRangeIndexed(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3));
	return 0;
}

static int lua_glDepthRange(lua_State *lua)
{
	glDepthRange(luaL_checknumber(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glDepthRangef(lua_State *lua)
{
	glDepthRangef(luaL_checknumber(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glViewportArrayv(lua_State *lua)
{
	const char *v = luaL_checkstring(lua, 3);
	glViewportArrayv(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		(const GLfloat *)v);
	return 0;
}

static int lua_glViewportIndexedf(lua_State *lua)
{
	glViewportIndexedf(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4),
		luaL_checknumber(lua, 5));
	return 0;
}

static int lua_glViewportIndexedfv(lua_State *lua)
{
	const void *v = luaL_checkstring(lua, 2);
	glViewportIndexedfv(luaL_checkinteger(lua, 1), (const GLfloat *)v);
	return 0;
}

static int lua_glViewport(lua_State *lua)
{
	glViewport(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

// RASTERIZATION.

// Multisampling.

static int lua_glGetMultisamplefv(lua_State *lua)
{
	GLfloat *val = (GLfloat*)malloc(sizeof(GLfloat) * 2);
	glGetMultisamplefv(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		val);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)val, sizeof(GLfloat) * 2);
	luaL_pushresult(&buffer);
	return 1;
}

static int lua_glMinSampleShading(lua_State *lua)
{
	glMinSampleShading(luaL_checknumber(lua, 1));
	return 0;
}

// Points.

static int lua_glPointSize(lua_State *lua)
{
	glPointSize(luaL_checknumber(lua, 1));
	return 0;
}

static int lua_glPointParameteri(lua_State *lua)
{
	glPointParameteri(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glPointParameterf(lua_State *lua)
{
	glPointParameterf(luaL_checkinteger(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

static int lua_glPointParameteriv(lua_State *lua)
{
	return 0;
}

static int lua_glPointParameterfv(lua_State *lua)
{
	return 0;
}

// Line Segments.

static int lua_glLineWidth(lua_State *lua)
{
	glLineWidth(luaL_checknumber(lua, 1));
	return 0;
}

// Polygons.

static int lua_glFrontFace(lua_State *lua)
{
	glFrontFace(luaL_checkinteger(lua, 1));
	return 0;
}

// Polygon Rast. & Depth Offset.

static int lua_glPolygonMode(lua_State *lua)
{
	glPolygonMode(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glPolygonOffset(lua_State *lua)
{
	glPolygonOffset(luaL_checknumber(lua, 1),
		luaL_checknumber(lua, 2));
	return 0;
}

// PER-FRAGMENT OPERATIONS.

static int lua_glScissorArrayv(lua_State *lua)
{
	const void *v = luaL_checkstring(lua, 3);
	glScissorArrayv(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		(const GLint*)v);
	return 0;
}

static int lua_glScissorIndexed(lua_State *lua)
{
	glScissorIndexed(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glScissorIndexedv(lua_State *lua)
{
	const void *v = luaL_checkstring(lua, 2);
	glScissorIndexedv(luaL_checkinteger(lua, 1), (const GLint *)v);
	return 0;
}

static int lua_glScissor(lua_State *lua)
{
	glScissor(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

// Multisample Fragment Ops.

static int lua_glSampleCoverage(lua_State *lua)
{
	glSampleCoverage(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glSampleMaski(lua_State *lua)
{
	glSampleMaski(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// Stencil Test.

static int lua_glStencilFunc(lua_State *lua)
{
	glStencilFunc(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glStencilFuncSeparate(lua_State *lua)
{
	glStencilFuncSeparate(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glStencilOp(lua_State *lua)
{
	glStencilOp(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glStencilOpSeparate(lua_State *lua)
{
	glStencilOpSeparate(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

// Depth Buffer Test.

static int lua_glDepthFunc(lua_State *lua)
{
	glDepthFunc(luaL_checkinteger(lua, 1));
	return 0;
}

// Occlusion Queries.

static int lua_glBeginQuery(lua_State *lua)
{
	return 0;
}

static int lua_glEndQuery(lua_State *lua)
{
	return 0;
}

// Blending.

static int lua_glBlendEquation(lua_State *lua)
{
	glBlendEquation(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glBlendEquationSeparate(lua_State *lua)
{
	glBlendEquationSeparate(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glBlendEquationi(lua_State *lua)
{
	glBlendEquationi(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glBlendEquationSeparatei(lua_State *lua)
{
	glBlendEquationSeparatei(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glBlendFunc(lua_State *lua)
{
	glBlendFunc(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

static int lua_glBlendFuncSeparate(lua_State *lua)
{
	glBlendFuncSeparate(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glBlendFunci(lua_State *lua)
{
	glBlendFunci(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3));
	return 0;
}

static int lua_glBlendFuncSeparatei(lua_State *lua)
{
	glBlendFuncSeparatei(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4),
		luaL_checkinteger(lua, 5));
	return 0;
}

static int lua_glBlendColor(lua_State *lua)
{
	glBlendColor(luaL_checknumber(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

// Dithering.

// Logical Operation.

static int lua_glLogicOp(lua_State *lua)
{
	glLogicOp(luaL_checkinteger(lua, 1));
	return 0;
}

// Hints.

static int lua_glHint(lua_State *lua)
{
	glHint(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// WHOLE FRAMEBUFFER

// Selecting Buffers for Writing.

static int lua_glDrawBuffer(lua_State *lua)
{
	glDrawBuffer(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glNamedFramebufferDrawBuffer(lua_State *lua)
{
	return 0;
}

static int lua_glDrawBuffers(lua_State *lua)
{
	glDrawBuffers(luaL_checkinteger(lua, 1), (const GLenum *)luaL_checkstring(lua, 2));
	return 0;
}

static int lua_glNamedFramebufferDrawBuffers(lua_State *lua)
{
	return 0;
}

// Fine Control of Buffer Updates.

static int lua_glColorMask(lua_State *lua)
{
	glColorMask(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2),
		luaL_checkinteger(lua, 3),
		luaL_checkinteger(lua, 4));
	return 0;
}

static int lua_glColorMaski(lua_State *lua)
{
	return 0;
}

static int lua_glDepthMask(lua_State *lua)
{
	glDepthMask(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glStencilMask(lua_State *lua)
{
	glStencilMask(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glStencilMaskSeparate(lua_State *lua)
{
	glStencilMaskSeparate(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// Clearing the Buffers.

static int lua_glClear(lua_State *lua)
{
	glClear(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glClearColor(lua_State *lua)
{
	glClearColor(luaL_checknumber(lua, 1),
		luaL_checknumber(lua, 2),
		luaL_checknumber(lua, 3),
		luaL_checknumber(lua, 4));
	return 0;
}

static int lua_glClearDepth(lua_State *lua)
{
	glClearDepth(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glClearDepthf(lua_State *lua)
{
	glClearDepthf(luaL_checknumber(lua, 1));
	return 0;
}

static int lua_glClearStencil(lua_State *lua)
{
	glClearStencil(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glClearBufferiv(lua_State *lua)
{
	return 0;
}

static int lua_glClearBufferfv(lua_State *lua)
{
	return 0;
}

static int lua_glClearBufferuiv(lua_State *lua)
{
	return 0;
}

static int lua_glClearNamedFramebufferiv(lua_State *lua)
{
	return 0;
}

static int lua_glClearNamedFramebufferfv(lua_State *lua)
{
	return 0;
}

static int lua_glClearNamedFramebufferuiv(lua_State *lua)
{
	return 0;
}

static int lua_glClearBufferfi(lua_State *lua)
{
	return 0;
}

static int lua_glClearNamedFramebufferfi(lua_State *lua)
{
	return 0;
}

// Invalidating Framebuffers.

static int lua_glInvalidateSubFramebuffer(lua_State *lua)
{
	return 0;
}

static int lua_glInvalidateNamedFramebufferSubData(lua_State *lua)
{
	return 0;
}

static int lua_glInvalidateFramebuffer(lua_State *lua)
{
	return 0;
}

static int lua_glInvalidateNamedFramebufferData(lua_State *lua)
{
	return 0;
}

// READING AND COPYING PIXELS.

// Reading Pixels.

static int lua_glReadBuffer(lua_State *lua)
{
	glReadBuffer(luaL_checkinteger(lua, 1));
	return 0;
}

static int lua_glNamedFramebufferReadBuffer(lua_State *lua)
{
	return 0;
}

static int lua_glReadPixels(lua_State *lua)
{
	GLint x = luaL_checkinteger(lua, 1);
	GLint y = luaL_checkinteger(lua, 2);
	GLsizei width = luaL_checkinteger(lua, 3);
	GLsizei height = luaL_checkinteger(lua, 4);
	GLenum format = luaL_checkinteger(lua, 5);
	GLenum type = luaL_checkinteger(lua, 6);
	GLsizei size = sizeof(char) * width * height * 4;
	char *data = (char*)malloc(size);
	glReadPixels(x, y, width, height, format, type, data);
	luaL_Buffer buffer;
	luaL_buffinit(lua, &buffer);
	luaL_addlstring(&buffer, (char*)data, size);
	luaL_pushresult(&buffer);
	return 0;
}

static int lua_glReadnPixels(lua_State *lua)
{
	return 0;
}

// Final Conversion.

static int lua_glClampColor(lua_State *lua)
{
	glClampColor(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	return 0;
}

// Copying Pixels.

static int lua_glBlitFramebuffer(lua_State *lua)
{
	return 0;
}

static int lua_glBlitNamedFramebuffer(lua_State *lua)
{
	return 0;
}

static int lua_glCopyImageSubData(lua_State *lua)
{
	return 0;
}

// STATE AND STATE REQUESTS.

// Simple Queries.

static int lua_glGetBooleani(lua_State *lua)
{
	return 0;
}

static int lua_glGetIntegeri(lua_State *lua)
{
	return 0;
}

static int lua_glGetFloati(lua_State *lua)
{
	return 0;
}

static int lua_glGetDoublei(lua_State *lua)
{
	return 0;
}

static int lua_glGetInteger64i(lua_State *lua)
{
	return 0;
}

static int lua_glGetBooleanv(lua_State *lua)
{
	return 0;
}

static int lua_glGetIntegerv(lua_State *lua)
{
	return 0;
}

static int lua_glGetInteger64v(lua_State *lua)
{
	return 0;
}

static int lua_glGetFloatv(lua_State *lua)
{
	return 0;
}

static int lua_glGetDoublev(lua_State *lua)
{
	return 0;
}

static int lua_glGetDoublei_v(lua_State *lua)
{
	return 0;
}

static int lua_glGetBooleani_v(lua_State *lua)
{
	return 0;
}

static int lua_glGetIntegeri_v(lua_State *lua)
{
	return 0;
}

static int lua_glGetFloati_v(lua_State *lua)
{
	return 0;
}

static int lua_glGetInteger64i_v(lua_State *lua)
{
	return 0;
}

// String Queries.

static int lua_glGetPointerv(lua_State *lua)
{
	return 0;
}

static int lua_glGetString(lua_State *lua)
{
	const char *string = (char *)glGetString(luaL_checkinteger(lua, 1));
	if (string) {
		lua_pushstring(lua, string);
	} else {
		lua_pushnil(lua);
	}
	return 1;
}

static int lua_glGetStringi(lua_State *lua)
{
	const char *string = (char *)glGetStringi(luaL_checkinteger(lua, 1),
		luaL_checkinteger(lua, 2));
	if (string) {
		lua_pushstring(lua, string);
	} else {
		lua_pushnil(lua);
	}
	return 1;
}

// Internal Format Queries.

static int lua_glGetInternalformativ(lua_State *lua)
{
	return 0;
}

static int lua_glGetInternalformati64v(lua_State *lua)
{
	return 0;
}

// Transform Feedback Queries.

static int lua_glGetTransformFeedbacki(lua_State *lua)
{
	return 0;
}

static int lua_glGetTransformFeedbacki64(lua_State *lua)
{
	return 0;
}

static int lua_glGetTransformFeedbackiv(lua_State *lua)
{
	return 0;
}

static int lua_glGetTransformFeedbacki_v(lua_State *lua)
{
	return 0;
}

static int lua_glGetTransformFeedbacki64_v(lua_State *lua)
{
	return 0;
}

int luaL_opengl(lua_State *lua)
{
	lua_newtable(lua);
	lua_setglobal(lua, "gl");
	lua_getglobal(lua, "gl");
	if (lua_istable(lua, -1)) {
		// Constants.
		lua_pushstring(lua, "VERSION_1_1");
		lua_pushinteger(lua, GL_VERSION_1_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_2");
		lua_pushinteger(lua, GL_VERSION_1_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_3");
		lua_pushinteger(lua, GL_VERSION_1_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_imaging");
		lua_pushinteger(lua, GL_ARB_imaging);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FALSE");
		lua_pushinteger(lua, GL_FALSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRUE");
		lua_pushinteger(lua, GL_TRUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BYTE");
		lua_pushinteger(lua, GL_BYTE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHORT");
		lua_pushinteger(lua, GL_SHORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT");
		lua_pushinteger(lua, GL_INT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT");
		lua_pushinteger(lua, GL_UNSIGNED_INT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT");
		lua_pushinteger(lua, GL_FLOAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2_BYTES");
		lua_pushinteger(lua, GL_2_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3_BYTES");
		lua_pushinteger(lua, GL_3_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4_BYTES");
		lua_pushinteger(lua, GL_4_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE");
		lua_pushinteger(lua, GL_DOUBLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINTS");
		lua_pushinteger(lua, GL_POINTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINES");
		lua_pushinteger(lua, GL_LINES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_LOOP");
		lua_pushinteger(lua, GL_LINE_LOOP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STRIP");
		lua_pushinteger(lua, GL_LINE_STRIP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLES");
		lua_pushinteger(lua, GL_TRIANGLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_STRIP");
		lua_pushinteger(lua, GL_TRIANGLE_STRIP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_FAN");
		lua_pushinteger(lua, GL_TRIANGLE_FAN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUADS");
		lua_pushinteger(lua, GL_QUADS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUAD_STRIP");
		lua_pushinteger(lua, GL_QUAD_STRIP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON");
		lua_pushinteger(lua, GL_POLYGON);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY");
		lua_pushinteger(lua, GL_VERTEX_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY");
		lua_pushinteger(lua, GL_NORMAL_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY");
		lua_pushinteger(lua, GL_COLOR_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY");
		lua_pushinteger(lua, GL_INDEX_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_SIZE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_TYPE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_TYPE");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_SIZE");
		lua_pushinteger(lua, GL_COLOR_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_TYPE");
		lua_pushinteger(lua, GL_COLOR_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_COLOR_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_TYPE");
		lua_pushinteger(lua, GL_INDEX_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_INDEX_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_POINTER");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_POINTER");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_POINTER");
		lua_pushinteger(lua, GL_COLOR_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_POINTER");
		lua_pushinteger(lua, GL_INDEX_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_POINTER");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_POINTER");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "V2F");
		lua_pushinteger(lua, GL_V2F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "V3F");
		lua_pushinteger(lua, GL_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C4UB_V2F");
		lua_pushinteger(lua, GL_C4UB_V2F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C4UB_V3F");
		lua_pushinteger(lua, GL_C4UB_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C3F_V3F");
		lua_pushinteger(lua, GL_C3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "N3F_V3F");
		lua_pushinteger(lua, GL_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C4F_N3F_V3F");
		lua_pushinteger(lua, GL_C4F_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_V3F");
		lua_pushinteger(lua, GL_T2F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T4F_V4F");
		lua_pushinteger(lua, GL_T4F_V4F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_C4UB_V3F");
		lua_pushinteger(lua, GL_T2F_C4UB_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_C3F_V3F");
		lua_pushinteger(lua, GL_T2F_C3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_N3F_V3F");
		lua_pushinteger(lua, GL_T2F_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_C4F_N3F_V3F");
		lua_pushinteger(lua, GL_T2F_C4F_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T4F_C4F_N3F_V4F");
		lua_pushinteger(lua, GL_T4F_C4F_N3F_V4F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_MODE");
		lua_pushinteger(lua, GL_MATRIX_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW");
		lua_pushinteger(lua, GL_MODELVIEW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROJECTION");
		lua_pushinteger(lua, GL_PROJECTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE");
		lua_pushinteger(lua, GL_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SMOOTH");
		lua_pushinteger(lua, GL_POINT_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE");
		lua_pushinteger(lua, GL_POINT_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_GRANULARITY");
		lua_pushinteger(lua, GL_POINT_SIZE_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_RANGE");
		lua_pushinteger(lua, GL_POINT_SIZE_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_SMOOTH");
		lua_pushinteger(lua, GL_LINE_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STIPPLE");
		lua_pushinteger(lua, GL_LINE_STIPPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STIPPLE_PATTERN");
		lua_pushinteger(lua, GL_LINE_STIPPLE_PATTERN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STIPPLE_REPEAT");
		lua_pushinteger(lua, GL_LINE_STIPPLE_REPEAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_WIDTH");
		lua_pushinteger(lua, GL_LINE_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_WIDTH_GRANULARITY");
		lua_pushinteger(lua, GL_LINE_WIDTH_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_WIDTH_RANGE");
		lua_pushinteger(lua, GL_LINE_WIDTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT");
		lua_pushinteger(lua, GL_POINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE");
		lua_pushinteger(lua, GL_LINE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FILL");
		lua_pushinteger(lua, GL_FILL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CW");
		lua_pushinteger(lua, GL_CW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CCW");
		lua_pushinteger(lua, GL_CCW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT");
		lua_pushinteger(lua, GL_FRONT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK");
		lua_pushinteger(lua, GL_BACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_MODE");
		lua_pushinteger(lua, GL_POLYGON_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_SMOOTH");
		lua_pushinteger(lua, GL_POLYGON_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_STIPPLE");
		lua_pushinteger(lua, GL_POLYGON_STIPPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG");
		lua_pushinteger(lua, GL_EDGE_FLAG);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_FACE");
		lua_pushinteger(lua, GL_CULL_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_FACE_MODE");
		lua_pushinteger(lua, GL_CULL_FACE_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_FACE");
		lua_pushinteger(lua, GL_FRONT_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_FACTOR");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_FACTOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_UNITS");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_POINT");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_POINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_LINE");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_LINE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_FILL");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_FILL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPILE");
		lua_pushinteger(lua, GL_COMPILE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPILE_AND_EXECUTE");
		lua_pushinteger(lua, GL_COMPILE_AND_EXECUTE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_BASE");
		lua_pushinteger(lua, GL_LIST_BASE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_INDEX");
		lua_pushinteger(lua, GL_LIST_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_MODE");
		lua_pushinteger(lua, GL_LIST_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEVER");
		lua_pushinteger(lua, GL_NEVER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LESS");
		lua_pushinteger(lua, GL_LESS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EQUAL");
		lua_pushinteger(lua, GL_EQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LEQUAL");
		lua_pushinteger(lua, GL_LEQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREATER");
		lua_pushinteger(lua, GL_GREATER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NOTEQUAL");
		lua_pushinteger(lua, GL_NOTEQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEQUAL");
		lua_pushinteger(lua, GL_GEQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALWAYS");
		lua_pushinteger(lua, GL_ALWAYS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_TEST");
		lua_pushinteger(lua, GL_DEPTH_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BITS");
		lua_pushinteger(lua, GL_DEPTH_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_CLEAR_VALUE");
		lua_pushinteger(lua, GL_DEPTH_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_FUNC");
		lua_pushinteger(lua, GL_DEPTH_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_RANGE");
		lua_pushinteger(lua, GL_DEPTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_WRITEMASK");
		lua_pushinteger(lua, GL_DEPTH_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHTING");
		lua_pushinteger(lua, GL_LIGHTING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT0");
		lua_pushinteger(lua, GL_LIGHT0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT1");
		lua_pushinteger(lua, GL_LIGHT1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT2");
		lua_pushinteger(lua, GL_LIGHT2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT3");
		lua_pushinteger(lua, GL_LIGHT3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT4");
		lua_pushinteger(lua, GL_LIGHT4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT5");
		lua_pushinteger(lua, GL_LIGHT5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT6");
		lua_pushinteger(lua, GL_LIGHT6);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT7");
		lua_pushinteger(lua, GL_LIGHT7);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPOT_EXPONENT");
		lua_pushinteger(lua, GL_SPOT_EXPONENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPOT_CUTOFF");
		lua_pushinteger(lua, GL_SPOT_CUTOFF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_ATTENUATION");
		lua_pushinteger(lua, GL_CONSTANT_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR_ATTENUATION");
		lua_pushinteger(lua, GL_LINEAR_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUADRATIC_ATTENUATION");
		lua_pushinteger(lua, GL_QUADRATIC_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AMBIENT");
		lua_pushinteger(lua, GL_AMBIENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DIFFUSE");
		lua_pushinteger(lua, GL_DIFFUSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPECULAR");
		lua_pushinteger(lua, GL_SPECULAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHININESS");
		lua_pushinteger(lua, GL_SHININESS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EMISSION");
		lua_pushinteger(lua, GL_EMISSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POSITION");
		lua_pushinteger(lua, GL_POSITION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPOT_DIRECTION");
		lua_pushinteger(lua, GL_SPOT_DIRECTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AMBIENT_AND_DIFFUSE");
		lua_pushinteger(lua, GL_AMBIENT_AND_DIFFUSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEXES");
		lua_pushinteger(lua, GL_COLOR_INDEXES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_TWO_SIDE");
		lua_pushinteger(lua, GL_LIGHT_MODEL_TWO_SIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_LOCAL_VIEWER");
		lua_pushinteger(lua, GL_LIGHT_MODEL_LOCAL_VIEWER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_AMBIENT");
		lua_pushinteger(lua, GL_LIGHT_MODEL_AMBIENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_AND_BACK");
		lua_pushinteger(lua, GL_FRONT_AND_BACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADE_MODEL");
		lua_pushinteger(lua, GL_SHADE_MODEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLAT");
		lua_pushinteger(lua, GL_FLAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH");
		lua_pushinteger(lua, GL_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATERIAL");
		lua_pushinteger(lua, GL_COLOR_MATERIAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATERIAL_FACE");
		lua_pushinteger(lua, GL_COLOR_MATERIAL_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATERIAL_PARAMETER");
		lua_pushinteger(lua, GL_COLOR_MATERIAL_PARAMETER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMALIZE");
		lua_pushinteger(lua, GL_NORMALIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE0");
		lua_pushinteger(lua, GL_CLIP_PLANE0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE1");
		lua_pushinteger(lua, GL_CLIP_PLANE1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE2");
		lua_pushinteger(lua, GL_CLIP_PLANE2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE3");
		lua_pushinteger(lua, GL_CLIP_PLANE3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE4");
		lua_pushinteger(lua, GL_CLIP_PLANE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE5");
		lua_pushinteger(lua, GL_CLIP_PLANE5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_RED_BITS");
		lua_pushinteger(lua, GL_ACCUM_RED_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_GREEN_BITS");
		lua_pushinteger(lua, GL_ACCUM_GREEN_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_BLUE_BITS");
		lua_pushinteger(lua, GL_ACCUM_BLUE_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_ALPHA_BITS");
		lua_pushinteger(lua, GL_ACCUM_ALPHA_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_CLEAR_VALUE");
		lua_pushinteger(lua, GL_ACCUM_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM");
		lua_pushinteger(lua, GL_ACCUM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD");
		lua_pushinteger(lua, GL_ADD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOAD");
		lua_pushinteger(lua, GL_LOAD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULT");
		lua_pushinteger(lua, GL_MULT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RETURN");
		lua_pushinteger(lua, GL_RETURN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_TEST");
		lua_pushinteger(lua, GL_ALPHA_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_TEST_REF");
		lua_pushinteger(lua, GL_ALPHA_TEST_REF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_TEST_FUNC");
		lua_pushinteger(lua, GL_ALPHA_TEST_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND");
		lua_pushinteger(lua, GL_BLEND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_SRC");
		lua_pushinteger(lua, GL_BLEND_SRC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_DST");
		lua_pushinteger(lua, GL_BLEND_DST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZERO");
		lua_pushinteger(lua, GL_ZERO);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE");
		lua_pushinteger(lua, GL_ONE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC_COLOR");
		lua_pushinteger(lua, GL_SRC_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_SRC_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_SRC_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC_ALPHA");
		lua_pushinteger(lua, GL_SRC_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_SRC_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_SRC_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DST_ALPHA");
		lua_pushinteger(lua, GL_DST_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_DST_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_DST_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DST_COLOR");
		lua_pushinteger(lua, GL_DST_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_DST_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_DST_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC_ALPHA_SATURATE");
		lua_pushinteger(lua, GL_SRC_ALPHA_SATURATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK");
		lua_pushinteger(lua, GL_FEEDBACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDER");
		lua_pushinteger(lua, GL_RENDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SELECT");
		lua_pushinteger(lua, GL_SELECT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2D");
		lua_pushinteger(lua, GL_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3D");
		lua_pushinteger(lua, GL_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3D_COLOR");
		lua_pushinteger(lua, GL_3D_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3D_COLOR_TEXTURE");
		lua_pushinteger(lua, GL_3D_COLOR_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4D_COLOR_TEXTURE");
		lua_pushinteger(lua, GL_4D_COLOR_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_TOKEN");
		lua_pushinteger(lua, GL_POINT_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_TOKEN");
		lua_pushinteger(lua, GL_LINE_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_RESET_TOKEN");
		lua_pushinteger(lua, GL_LINE_RESET_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_TOKEN");
		lua_pushinteger(lua, GL_POLYGON_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BITMAP_TOKEN");
		lua_pushinteger(lua, GL_BITMAP_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_PIXEL_TOKEN");
		lua_pushinteger(lua, GL_DRAW_PIXEL_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY_PIXEL_TOKEN");
		lua_pushinteger(lua, GL_COPY_PIXEL_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PASS_THROUGH_TOKEN");
		lua_pushinteger(lua, GL_PASS_THROUGH_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK_BUFFER_POINTER");
		lua_pushinteger(lua, GL_FEEDBACK_BUFFER_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK_BUFFER_SIZE");
		lua_pushinteger(lua, GL_FEEDBACK_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK_BUFFER_TYPE");
		lua_pushinteger(lua, GL_FEEDBACK_BUFFER_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SELECTION_BUFFER_POINTER");
		lua_pushinteger(lua, GL_SELECTION_BUFFER_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SELECTION_BUFFER_SIZE");
		lua_pushinteger(lua, GL_SELECTION_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG");
		lua_pushinteger(lua, GL_FOG);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_MODE");
		lua_pushinteger(lua, GL_FOG_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_DENSITY");
		lua_pushinteger(lua, GL_FOG_DENSITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COLOR");
		lua_pushinteger(lua, GL_FOG_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_INDEX");
		lua_pushinteger(lua, GL_FOG_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_START");
		lua_pushinteger(lua, GL_FOG_START);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_END");
		lua_pushinteger(lua, GL_FOG_END);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR");
		lua_pushinteger(lua, GL_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXP");
		lua_pushinteger(lua, GL_EXP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXP2");
		lua_pushinteger(lua, GL_EXP2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOGIC_OP");
		lua_pushinteger(lua, GL_LOGIC_OP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_LOGIC_OP");
		lua_pushinteger(lua, GL_INDEX_LOGIC_OP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_LOGIC_OP");
		lua_pushinteger(lua, GL_COLOR_LOGIC_OP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOGIC_OP_MODE");
		lua_pushinteger(lua, GL_LOGIC_OP_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLEAR");
		lua_pushinteger(lua, GL_CLEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SET");
		lua_pushinteger(lua, GL_SET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY");
		lua_pushinteger(lua, GL_COPY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY_INVERTED");
		lua_pushinteger(lua, GL_COPY_INVERTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NOOP");
		lua_pushinteger(lua, GL_NOOP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVERT");
		lua_pushinteger(lua, GL_INVERT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AND");
		lua_pushinteger(lua, GL_AND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NAND");
		lua_pushinteger(lua, GL_NAND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OR");
		lua_pushinteger(lua, GL_OR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NOR");
		lua_pushinteger(lua, GL_NOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "XOR");
		lua_pushinteger(lua, GL_XOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EQUIV");
		lua_pushinteger(lua, GL_EQUIV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AND_REVERSE");
		lua_pushinteger(lua, GL_AND_REVERSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AND_INVERTED");
		lua_pushinteger(lua, GL_AND_INVERTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OR_REVERSE");
		lua_pushinteger(lua, GL_OR_REVERSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OR_INVERTED");
		lua_pushinteger(lua, GL_OR_INVERTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BITS");
		lua_pushinteger(lua, GL_STENCIL_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_TEST");
		lua_pushinteger(lua, GL_STENCIL_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_CLEAR_VALUE");
		lua_pushinteger(lua, GL_STENCIL_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_FUNC");
		lua_pushinteger(lua, GL_STENCIL_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_VALUE_MASK");
		lua_pushinteger(lua, GL_STENCIL_VALUE_MASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_FAIL");
		lua_pushinteger(lua, GL_STENCIL_FAIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_PASS_DEPTH_FAIL");
		lua_pushinteger(lua, GL_STENCIL_PASS_DEPTH_FAIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_PASS_DEPTH_PASS");
		lua_pushinteger(lua, GL_STENCIL_PASS_DEPTH_PASS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_REF");
		lua_pushinteger(lua, GL_STENCIL_REF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_WRITEMASK");
		lua_pushinteger(lua, GL_STENCIL_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX");
		lua_pushinteger(lua, GL_STENCIL_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "KEEP");
		lua_pushinteger(lua, GL_KEEP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACE");
		lua_pushinteger(lua, GL_REPLACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INCR");
		lua_pushinteger(lua, GL_INCR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DECR");
		lua_pushinteger(lua, GL_DECR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NONE");
		lua_pushinteger(lua, GL_NONE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LEFT");
		lua_pushinteger(lua, GL_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RIGHT");
		lua_pushinteger(lua, GL_RIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_LEFT");
		lua_pushinteger(lua, GL_FRONT_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_RIGHT");
		lua_pushinteger(lua, GL_FRONT_RIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_LEFT");
		lua_pushinteger(lua, GL_BACK_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_RIGHT");
		lua_pushinteger(lua, GL_BACK_RIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX0");
		lua_pushinteger(lua, GL_AUX0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX1");
		lua_pushinteger(lua, GL_AUX1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX2");
		lua_pushinteger(lua, GL_AUX2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX3");
		lua_pushinteger(lua, GL_AUX3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX");
		lua_pushinteger(lua, GL_COLOR_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED");
		lua_pushinteger(lua, GL_RED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN");
		lua_pushinteger(lua, GL_GREEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE");
		lua_pushinteger(lua, GL_BLUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA");
		lua_pushinteger(lua, GL_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE");
		lua_pushinteger(lua, GL_LUMINANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_BITS");
		lua_pushinteger(lua, GL_ALPHA_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_BITS");
		lua_pushinteger(lua, GL_RED_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_BITS");
		lua_pushinteger(lua, GL_GREEN_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_BITS");
		lua_pushinteger(lua, GL_BLUE_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_BITS");
		lua_pushinteger(lua, GL_INDEX_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUBPIXEL_BITS");
		lua_pushinteger(lua, GL_SUBPIXEL_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX_BUFFERS");
		lua_pushinteger(lua, GL_AUX_BUFFERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_BUFFER");
		lua_pushinteger(lua, GL_READ_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER");
		lua_pushinteger(lua, GL_DRAW_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLEBUFFER");
		lua_pushinteger(lua, GL_DOUBLEBUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STEREO");
		lua_pushinteger(lua, GL_STEREO);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BITMAP");
		lua_pushinteger(lua, GL_BITMAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR");
		lua_pushinteger(lua, GL_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH");
		lua_pushinteger(lua, GL_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL");
		lua_pushinteger(lua, GL_STENCIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DITHER");
		lua_pushinteger(lua, GL_DITHER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB");
		lua_pushinteger(lua, GL_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA");
		lua_pushinteger(lua, GL_RGBA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_LIST_NESTING");
		lua_pushinteger(lua, GL_MAX_LIST_NESTING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_EVAL_ORDER");
		lua_pushinteger(lua, GL_MAX_EVAL_ORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_LIGHTS");
		lua_pushinteger(lua, GL_MAX_LIGHTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CLIP_PLANES");
		lua_pushinteger(lua, GL_MAX_CLIP_PLANES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PIXEL_MAP_TABLE");
		lua_pushinteger(lua, GL_MAX_PIXEL_MAP_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_MODELVIEW_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_MODELVIEW_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_NAME_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_NAME_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROJECTION_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_PROJECTION_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_TEXTURE_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VIEWPORT_DIMS");
		lua_pushinteger(lua, GL_MAX_VIEWPORT_DIMS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CLIENT_ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_CLIENT_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_CLEAR_VALUE");
		lua_pushinteger(lua, GL_COLOR_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_WRITEMASK");
		lua_pushinteger(lua, GL_COLOR_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_INDEX");
		lua_pushinteger(lua, GL_CURRENT_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_COLOR");
		lua_pushinteger(lua, GL_CURRENT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_NORMAL");
		lua_pushinteger(lua, GL_CURRENT_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_COLOR");
		lua_pushinteger(lua, GL_CURRENT_RASTER_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_DISTANCE");
		lua_pushinteger(lua, GL_CURRENT_RASTER_DISTANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_INDEX");
		lua_pushinteger(lua, GL_CURRENT_RASTER_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_POSITION");
		lua_pushinteger(lua, GL_CURRENT_RASTER_POSITION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_TEXTURE_COORDS");
		lua_pushinteger(lua, GL_CURRENT_RASTER_TEXTURE_COORDS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_POSITION_VALID");
		lua_pushinteger(lua, GL_CURRENT_RASTER_POSITION_VALID);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_TEXTURE_COORDS");
		lua_pushinteger(lua, GL_CURRENT_TEXTURE_COORDS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_CLEAR_VALUE");
		lua_pushinteger(lua, GL_INDEX_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_MODE");
		lua_pushinteger(lua, GL_INDEX_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_WRITEMASK");
		lua_pushinteger(lua, GL_INDEX_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW_MATRIX");
		lua_pushinteger(lua, GL_MODELVIEW_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW_STACK_DEPTH");
		lua_pushinteger(lua, GL_MODELVIEW_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NAME_STACK_DEPTH");
		lua_pushinteger(lua, GL_NAME_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROJECTION_MATRIX");
		lua_pushinteger(lua, GL_PROJECTION_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROJECTION_STACK_DEPTH");
		lua_pushinteger(lua, GL_PROJECTION_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDER_MODE");
		lua_pushinteger(lua, GL_RENDER_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_MODE");
		lua_pushinteger(lua, GL_RGBA_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MATRIX");
		lua_pushinteger(lua, GL_TEXTURE_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_STACK_DEPTH");
		lua_pushinteger(lua, GL_TEXTURE_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT");
		lua_pushinteger(lua, GL_VIEWPORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUTO_NORMAL");
		lua_pushinteger(lua, GL_AUTO_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_COLOR_4");
		lua_pushinteger(lua, GL_MAP1_COLOR_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_INDEX");
		lua_pushinteger(lua, GL_MAP1_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_NORMAL");
		lua_pushinteger(lua, GL_MAP1_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_1");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_2");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_3");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_4");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_3");
		lua_pushinteger(lua, GL_MAP1_VERTEX_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_4");
		lua_pushinteger(lua, GL_MAP1_VERTEX_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_COLOR_4");
		lua_pushinteger(lua, GL_MAP2_COLOR_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_INDEX");
		lua_pushinteger(lua, GL_MAP2_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_NORMAL");
		lua_pushinteger(lua, GL_MAP2_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_1");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_2");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_3");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_4");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_3");
		lua_pushinteger(lua, GL_MAP2_VERTEX_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_4");
		lua_pushinteger(lua, GL_MAP2_VERTEX_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_GRID_DOMAIN");
		lua_pushinteger(lua, GL_MAP1_GRID_DOMAIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_GRID_SEGMENTS");
		lua_pushinteger(lua, GL_MAP1_GRID_SEGMENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_GRID_DOMAIN");
		lua_pushinteger(lua, GL_MAP2_GRID_DOMAIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_GRID_SEGMENTS");
		lua_pushinteger(lua, GL_MAP2_GRID_SEGMENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COEFF");
		lua_pushinteger(lua, GL_COEFF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ORDER");
		lua_pushinteger(lua, GL_ORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOMAIN");
		lua_pushinteger(lua, GL_DOMAIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERSPECTIVE_CORRECTION_HINT");
		lua_pushinteger(lua, GL_PERSPECTIVE_CORRECTION_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SMOOTH_HINT");
		lua_pushinteger(lua, GL_POINT_SMOOTH_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_SMOOTH_HINT");
		lua_pushinteger(lua, GL_LINE_SMOOTH_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_SMOOTH_HINT");
		lua_pushinteger(lua, GL_POLYGON_SMOOTH_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_HINT");
		lua_pushinteger(lua, GL_FOG_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DONT_CARE");
		lua_pushinteger(lua, GL_DONT_CARE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FASTEST");
		lua_pushinteger(lua, GL_FASTEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NICEST");
		lua_pushinteger(lua, GL_NICEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_BOX");
		lua_pushinteger(lua, GL_SCISSOR_BOX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_TEST");
		lua_pushinteger(lua, GL_SCISSOR_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_COLOR");
		lua_pushinteger(lua, GL_MAP_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_STENCIL");
		lua_pushinteger(lua, GL_MAP_STENCIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_SHIFT");
		lua_pushinteger(lua, GL_INDEX_SHIFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_OFFSET");
		lua_pushinteger(lua, GL_INDEX_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_SCALE");
		lua_pushinteger(lua, GL_RED_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_BIAS");
		lua_pushinteger(lua, GL_RED_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_SCALE");
		lua_pushinteger(lua, GL_GREEN_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_BIAS");
		lua_pushinteger(lua, GL_GREEN_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_SCALE");
		lua_pushinteger(lua, GL_BLUE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_BIAS");
		lua_pushinteger(lua, GL_BLUE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_SCALE");
		lua_pushinteger(lua, GL_ALPHA_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_BIAS");
		lua_pushinteger(lua, GL_ALPHA_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_SCALE");
		lua_pushinteger(lua, GL_DEPTH_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BIAS");
		lua_pushinteger(lua, GL_DEPTH_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_S_TO_S_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_S_TO_S_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_I_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_I_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_R_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_R_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_G_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_G_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_B_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_B_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_A_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_A_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_R_TO_R_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_R_TO_R_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_G_TO_G_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_G_TO_G_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_B_TO_B_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_B_TO_B_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_A_TO_A_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_A_TO_A_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_S_TO_S");
		lua_pushinteger(lua, GL_PIXEL_MAP_S_TO_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_I");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_R");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_G");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_G);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_B");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_B);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_A");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_A);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_R_TO_R");
		lua_pushinteger(lua, GL_PIXEL_MAP_R_TO_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_G_TO_G");
		lua_pushinteger(lua, GL_PIXEL_MAP_G_TO_G);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_B_TO_B");
		lua_pushinteger(lua, GL_PIXEL_MAP_B_TO_B);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_A_TO_A");
		lua_pushinteger(lua, GL_PIXEL_MAP_A_TO_A);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_ALIGNMENT");
		lua_pushinteger(lua, GL_PACK_ALIGNMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_LSB_FIRST");
		lua_pushinteger(lua, GL_PACK_LSB_FIRST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_ROW_LENGTH");
		lua_pushinteger(lua, GL_PACK_ROW_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_PIXELS");
		lua_pushinteger(lua, GL_PACK_SKIP_PIXELS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_ROWS");
		lua_pushinteger(lua, GL_PACK_SKIP_ROWS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SWAP_BYTES");
		lua_pushinteger(lua, GL_PACK_SWAP_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_ALIGNMENT");
		lua_pushinteger(lua, GL_UNPACK_ALIGNMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_LSB_FIRST");
		lua_pushinteger(lua, GL_UNPACK_LSB_FIRST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_ROW_LENGTH");
		lua_pushinteger(lua, GL_UNPACK_ROW_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_PIXELS");
		lua_pushinteger(lua, GL_UNPACK_SKIP_PIXELS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_ROWS");
		lua_pushinteger(lua, GL_UNPACK_SKIP_ROWS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SWAP_BYTES");
		lua_pushinteger(lua, GL_UNPACK_SWAP_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZOOM_X");
		lua_pushinteger(lua, GL_ZOOM_X);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZOOM_Y");
		lua_pushinteger(lua, GL_ZOOM_Y);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ENV");
		lua_pushinteger(lua, GL_TEXTURE_ENV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ENV_MODE");
		lua_pushinteger(lua, GL_TEXTURE_ENV_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D");
		lua_pushinteger(lua, GL_TEXTURE_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D");
		lua_pushinteger(lua, GL_TEXTURE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_S");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_T");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_T);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAG_FILTER");
		lua_pushinteger(lua, GL_TEXTURE_MAG_FILTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MIN_FILTER");
		lua_pushinteger(lua, GL_TEXTURE_MIN_FILTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ENV_COLOR");
		lua_pushinteger(lua, GL_TEXTURE_ENV_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_S");
		lua_pushinteger(lua, GL_TEXTURE_GEN_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_T");
		lua_pushinteger(lua, GL_TEXTURE_GEN_T);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_R");
		lua_pushinteger(lua, GL_TEXTURE_GEN_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_Q");
		lua_pushinteger(lua, GL_TEXTURE_GEN_Q);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_MODE");
		lua_pushinteger(lua, GL_TEXTURE_GEN_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BORDER_COLOR");
		lua_pushinteger(lua, GL_TEXTURE_BORDER_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WIDTH");
		lua_pushinteger(lua, GL_TEXTURE_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_HEIGHT");
		lua_pushinteger(lua, GL_TEXTURE_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BORDER");
		lua_pushinteger(lua, GL_TEXTURE_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPONENTS");
		lua_pushinteger(lua, GL_TEXTURE_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RED_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GREEN_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BLUE_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ALPHA_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LUMINANCE_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_LUMINANCE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTENSITY_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_INTENSITY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEAREST_MIPMAP_NEAREST");
		lua_pushinteger(lua, GL_NEAREST_MIPMAP_NEAREST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEAREST_MIPMAP_LINEAR");
		lua_pushinteger(lua, GL_NEAREST_MIPMAP_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR_MIPMAP_NEAREST");
		lua_pushinteger(lua, GL_LINEAR_MIPMAP_NEAREST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR_MIPMAP_LINEAR");
		lua_pushinteger(lua, GL_LINEAR_MIPMAP_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_LINEAR");
		lua_pushinteger(lua, GL_OBJECT_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_PLANE");
		lua_pushinteger(lua, GL_OBJECT_PLANE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_LINEAR");
		lua_pushinteger(lua, GL_EYE_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_PLANE");
		lua_pushinteger(lua, GL_EYE_PLANE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPHERE_MAP");
		lua_pushinteger(lua, GL_SPHERE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DECAL");
		lua_pushinteger(lua, GL_DECAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODULATE");
		lua_pushinteger(lua, GL_MODULATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEAREST");
		lua_pushinteger(lua, GL_NEAREST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPEAT");
		lua_pushinteger(lua, GL_REPEAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP");
		lua_pushinteger(lua, GL_CLAMP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "S");
		lua_pushinteger(lua, GL_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T");
		lua_pushinteger(lua, GL_T);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R");
		lua_pushinteger(lua, GL_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Q");
		lua_pushinteger(lua, GL_Q);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VENDOR");
		lua_pushinteger(lua, GL_VENDOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERER");
		lua_pushinteger(lua, GL_RENDERER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION");
		lua_pushinteger(lua, GL_VERSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXTENSIONS");
		lua_pushinteger(lua, GL_EXTENSIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NO_ERROR");
		lua_pushinteger(lua, GL_NO_ERROR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_ENUM");
		lua_pushinteger(lua, GL_INVALID_ENUM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_VALUE");
		lua_pushinteger(lua, GL_INVALID_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_OPERATION");
		lua_pushinteger(lua, GL_INVALID_OPERATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STACK_OVERFLOW");
		lua_pushinteger(lua, GL_STACK_OVERFLOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STACK_UNDERFLOW");
		lua_pushinteger(lua, GL_STACK_UNDERFLOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUT_OF_MEMORY");
		lua_pushinteger(lua, GL_OUT_OF_MEMORY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_BIT");
		lua_pushinteger(lua, GL_CURRENT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_BIT");
		lua_pushinteger(lua, GL_POINT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_BIT");
		lua_pushinteger(lua, GL_LINE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_BIT");
		lua_pushinteger(lua, GL_POLYGON_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_STIPPLE_BIT");
		lua_pushinteger(lua, GL_POLYGON_STIPPLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MODE_BIT");
		lua_pushinteger(lua, GL_PIXEL_MODE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHTING_BIT");
		lua_pushinteger(lua, GL_LIGHTING_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_BIT");
		lua_pushinteger(lua, GL_FOG_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BUFFER_BIT");
		lua_pushinteger(lua, GL_DEPTH_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_BUFFER_BIT");
		lua_pushinteger(lua, GL_ACCUM_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BUFFER_BIT");
		lua_pushinteger(lua, GL_STENCIL_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT_BIT");
		lua_pushinteger(lua, GL_VIEWPORT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_BIT");
		lua_pushinteger(lua, GL_TRANSFORM_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ENABLE_BIT");
		lua_pushinteger(lua, GL_ENABLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_BUFFER_BIT");
		lua_pushinteger(lua, GL_COLOR_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HINT_BIT");
		lua_pushinteger(lua, GL_HINT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_BIT");
		lua_pushinteger(lua, GL_EVAL_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_BIT");
		lua_pushinteger(lua, GL_LIST_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BIT");
		lua_pushinteger(lua, GL_TEXTURE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_BIT");
		lua_pushinteger(lua, GL_SCISSOR_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALL_ATTRIB_BITS");
		lua_pushinteger(lua, GL_ALL_ATTRIB_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_PRIORITY");
		lua_pushinteger(lua, GL_TEXTURE_PRIORITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RESIDENT");
		lua_pushinteger(lua, GL_TEXTURE_RESIDENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_1D");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_2D");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTERNAL_FORMAT");
		lua_pushinteger(lua, GL_TEXTURE_INTERNAL_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA4");
		lua_pushinteger(lua, GL_ALPHA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA8");
		lua_pushinteger(lua, GL_ALPHA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA12");
		lua_pushinteger(lua, GL_ALPHA12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16");
		lua_pushinteger(lua, GL_ALPHA16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE4");
		lua_pushinteger(lua, GL_LUMINANCE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8");
		lua_pushinteger(lua, GL_LUMINANCE8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12");
		lua_pushinteger(lua, GL_LUMINANCE12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16");
		lua_pushinteger(lua, GL_LUMINANCE16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE4_ALPHA4");
		lua_pushinteger(lua, GL_LUMINANCE4_ALPHA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE6_ALPHA2");
		lua_pushinteger(lua, GL_LUMINANCE6_ALPHA2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8_ALPHA8");
		lua_pushinteger(lua, GL_LUMINANCE8_ALPHA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_ALPHA4");
		lua_pushinteger(lua, GL_LUMINANCE12_ALPHA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_ALPHA12");
		lua_pushinteger(lua, GL_LUMINANCE12_ALPHA12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16_ALPHA16");
		lua_pushinteger(lua, GL_LUMINANCE16_ALPHA16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY");
		lua_pushinteger(lua, GL_INTENSITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY4");
		lua_pushinteger(lua, GL_INTENSITY4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY8");
		lua_pushinteger(lua, GL_INTENSITY8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY12");
		lua_pushinteger(lua, GL_INTENSITY12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16");
		lua_pushinteger(lua, GL_INTENSITY16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R3_G3_B2");
		lua_pushinteger(lua, GL_R3_G3_B2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB4");
		lua_pushinteger(lua, GL_RGB4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB5");
		lua_pushinteger(lua, GL_RGB5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8");
		lua_pushinteger(lua, GL_RGB8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10");
		lua_pushinteger(lua, GL_RGB10);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB12");
		lua_pushinteger(lua, GL_RGB12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16");
		lua_pushinteger(lua, GL_RGB16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA2");
		lua_pushinteger(lua, GL_RGBA2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA4");
		lua_pushinteger(lua, GL_RGBA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB5_A1");
		lua_pushinteger(lua, GL_RGB5_A1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8");
		lua_pushinteger(lua, GL_RGBA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10_A2");
		lua_pushinteger(lua, GL_RGB10_A2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA12");
		lua_pushinteger(lua, GL_RGBA12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16");
		lua_pushinteger(lua, GL_RGBA16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_PIXEL_STORE_BIT");
		lua_pushinteger(lua, GL_CLIENT_PIXEL_STORE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_VERTEX_ARRAY_BIT");
		lua_pushinteger(lua, GL_CLIENT_VERTEX_ARRAY_BIT);
		lua_settable(lua, -3);
		//lua_pushstring(lua, "ALL_CLIENT_ATTRIB_BITS");
		//lua_pushinteger(lua, GL_ALL_CLIENT_ATTRIB_BITS);
		//lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ALL_ATTRIB_BITS");
		lua_pushinteger(lua, GL_CLIENT_ALL_ATTRIB_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESCALE_NORMAL");
		lua_pushinteger(lua, GL_RESCALE_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_EDGE");
		lua_pushinteger(lua, GL_CLAMP_TO_EDGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ELEMENTS_VERTICES");
		lua_pushinteger(lua, GL_MAX_ELEMENTS_VERTICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ELEMENTS_INDICES");
		lua_pushinteger(lua, GL_MAX_ELEMENTS_INDICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGR");
		lua_pushinteger(lua, GL_BGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA");
		lua_pushinteger(lua, GL_BGRA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE_3_3_2");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE_3_3_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE_2_3_3_REV");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE_2_3_3_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_6_5");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_6_5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_6_5_REV");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_6_5_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_4_4_4_4");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_4_4_4_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_4_4_4_4_REV");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_4_4_4_4_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_5_5_1");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_5_5_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_1_5_5_5_REV");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_1_5_5_5_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_8_8");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_8_8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_8_8_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_8_8_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_10_10_10_2");
		lua_pushinteger(lua, GL_UNSIGNED_INT_10_10_10_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_2_10_10_10_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_2_10_10_10_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_COLOR_CONTROL");
		lua_pushinteger(lua, GL_LIGHT_MODEL_COLOR_CONTROL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SINGLE_COLOR");
		lua_pushinteger(lua, GL_SINGLE_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARATE_SPECULAR_COLOR");
		lua_pushinteger(lua, GL_SEPARATE_SPECULAR_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MIN_LOD");
		lua_pushinteger(lua, GL_TEXTURE_MIN_LOD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_LOD");
		lua_pushinteger(lua, GL_TEXTURE_MAX_LOD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BASE_LEVEL");
		lua_pushinteger(lua, GL_TEXTURE_BASE_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_LEVEL");
		lua_pushinteger(lua, GL_TEXTURE_MAX_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_POINT_SIZE_RANGE");
		lua_pushinteger(lua, GL_SMOOTH_POINT_SIZE_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_POINT_SIZE_GRANULARITY");
		lua_pushinteger(lua, GL_SMOOTH_POINT_SIZE_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_LINE_WIDTH_RANGE");
		lua_pushinteger(lua, GL_SMOOTH_LINE_WIDTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_LINE_WIDTH_GRANULARITY");
		lua_pushinteger(lua, GL_SMOOTH_LINE_WIDTH_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALIASED_POINT_SIZE_RANGE");
		lua_pushinteger(lua, GL_ALIASED_POINT_SIZE_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALIASED_LINE_WIDTH_RANGE");
		lua_pushinteger(lua, GL_ALIASED_LINE_WIDTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_IMAGES");
		lua_pushinteger(lua, GL_PACK_SKIP_IMAGES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_IMAGE_HEIGHT");
		lua_pushinteger(lua, GL_PACK_IMAGE_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_IMAGES");
		lua_pushinteger(lua, GL_UNPACK_SKIP_IMAGES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_IMAGE_HEIGHT");
		lua_pushinteger(lua, GL_UNPACK_IMAGE_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_3D");
		lua_pushinteger(lua, GL_TEXTURE_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_3D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_R");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_3D_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_3D_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_3D");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_COLOR");
		lua_pushinteger(lua, GL_CONSTANT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_CONSTANT_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_CONSTANT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_ALPHA");
		lua_pushinteger(lua, GL_CONSTANT_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_CONSTANT_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_CONSTANT_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE");
		lua_pushinteger(lua, GL_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_COLOR_TABLE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_COLOR_TABLE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_COLOR_TABLE");
		lua_pushinteger(lua, GL_PROXY_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_POST_CONVOLUTION_COLOR_TABLE");
		lua_pushinteger(lua, GL_PROXY_POST_CONVOLUTION_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_POST_COLOR_MATRIX_COLOR_TABLE");
		lua_pushinteger(lua, GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_SCALE");
		lua_pushinteger(lua, GL_COLOR_TABLE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_BIAS");
		lua_pushinteger(lua, GL_COLOR_TABLE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_FORMAT");
		lua_pushinteger(lua, GL_COLOR_TABLE_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_WIDTH");
		lua_pushinteger(lua, GL_COLOR_TABLE_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_RED_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_GREEN_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_BLUE_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_ALPHA_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_LUMINANCE_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_LUMINANCE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_INTENSITY_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_INTENSITY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_1D");
		lua_pushinteger(lua, GL_CONVOLUTION_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_2D");
		lua_pushinteger(lua, GL_CONVOLUTION_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARABLE_2D");
		lua_pushinteger(lua, GL_SEPARABLE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_BORDER_MODE");
		lua_pushinteger(lua, GL_CONVOLUTION_BORDER_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FILTER_SCALE");
		lua_pushinteger(lua, GL_CONVOLUTION_FILTER_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FILTER_BIAS");
		lua_pushinteger(lua, GL_CONVOLUTION_FILTER_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REDUCE");
		lua_pushinteger(lua, GL_REDUCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FORMAT");
		lua_pushinteger(lua, GL_CONVOLUTION_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_WIDTH");
		lua_pushinteger(lua, GL_CONVOLUTION_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_HEIGHT");
		lua_pushinteger(lua, GL_CONVOLUTION_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CONVOLUTION_WIDTH");
		lua_pushinteger(lua, GL_MAX_CONVOLUTION_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CONVOLUTION_HEIGHT");
		lua_pushinteger(lua, GL_MAX_CONVOLUTION_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_RED_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_RED_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_GREEN_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_GREEN_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_BLUE_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_BLUE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_ALPHA_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_ALPHA_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_RED_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_RED_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_GREEN_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_GREEN_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_BLUE_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_BLUE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_ALPHA_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_ALPHA_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_BORDER");
		lua_pushinteger(lua, GL_CONSTANT_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLICATE_BORDER");
		lua_pushinteger(lua, GL_REPLICATE_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_BORDER_COLOR");
		lua_pushinteger(lua, GL_CONVOLUTION_BORDER_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATRIX");
		lua_pushinteger(lua, GL_COLOR_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATRIX_STACK_DEPTH");
		lua_pushinteger(lua, GL_COLOR_MATRIX_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COLOR_MATRIX_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_COLOR_MATRIX_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_RED_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_RED_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_GREEN_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_GREEN_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_BLUE_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_BLUE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_ALPHA_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_ALPHA_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_RED_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_RED_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_GREEN_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_GREEN_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_BLUE_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_BLUE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_ALPHA_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_ALPHA_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM");
		lua_pushinteger(lua, GL_HISTOGRAM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_HISTOGRAM");
		lua_pushinteger(lua, GL_PROXY_HISTOGRAM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_WIDTH");
		lua_pushinteger(lua, GL_HISTOGRAM_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_FORMAT");
		lua_pushinteger(lua, GL_HISTOGRAM_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_RED_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_GREEN_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_BLUE_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_ALPHA_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_LUMINANCE_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_LUMINANCE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_SINK");
		lua_pushinteger(lua, GL_HISTOGRAM_SINK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX");
		lua_pushinteger(lua, GL_MINMAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_FORMAT");
		lua_pushinteger(lua, GL_MINMAX_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_SINK");
		lua_pushinteger(lua, GL_MINMAX_SINK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TABLE_TOO_LARGE");
		lua_pushinteger(lua, GL_TABLE_TOO_LARGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION");
		lua_pushinteger(lua, GL_BLEND_EQUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN");
		lua_pushinteger(lua, GL_MIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX");
		lua_pushinteger(lua, GL_MAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_ADD");
		lua_pushinteger(lua, GL_FUNC_ADD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_SUBTRACT");
		lua_pushinteger(lua, GL_FUNC_SUBTRACT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_REVERSE_SUBTRACT");
		lua_pushinteger(lua, GL_FUNC_REVERSE_SUBTRACT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_COLOR");
		lua_pushinteger(lua, GL_BLEND_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE0");
		lua_pushinteger(lua, GL_TEXTURE0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE1");
		lua_pushinteger(lua, GL_TEXTURE1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE2");
		lua_pushinteger(lua, GL_TEXTURE2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE3");
		lua_pushinteger(lua, GL_TEXTURE3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE4");
		lua_pushinteger(lua, GL_TEXTURE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE5");
		lua_pushinteger(lua, GL_TEXTURE5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE6");
		lua_pushinteger(lua, GL_TEXTURE6);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE7");
		lua_pushinteger(lua, GL_TEXTURE7);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE8");
		lua_pushinteger(lua, GL_TEXTURE8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE9");
		lua_pushinteger(lua, GL_TEXTURE9);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE10");
		lua_pushinteger(lua, GL_TEXTURE10);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE11");
		lua_pushinteger(lua, GL_TEXTURE11);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE12");
		lua_pushinteger(lua, GL_TEXTURE12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE13");
		lua_pushinteger(lua, GL_TEXTURE13);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE14");
		lua_pushinteger(lua, GL_TEXTURE14);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE15");
		lua_pushinteger(lua, GL_TEXTURE15);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE16");
		lua_pushinteger(lua, GL_TEXTURE16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE17");
		lua_pushinteger(lua, GL_TEXTURE17);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE18");
		lua_pushinteger(lua, GL_TEXTURE18);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE19");
		lua_pushinteger(lua, GL_TEXTURE19);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE20");
		lua_pushinteger(lua, GL_TEXTURE20);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE21");
		lua_pushinteger(lua, GL_TEXTURE21);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE22");
		lua_pushinteger(lua, GL_TEXTURE22);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE23");
		lua_pushinteger(lua, GL_TEXTURE23);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE24");
		lua_pushinteger(lua, GL_TEXTURE24);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE25");
		lua_pushinteger(lua, GL_TEXTURE25);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE26");
		lua_pushinteger(lua, GL_TEXTURE26);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE27");
		lua_pushinteger(lua, GL_TEXTURE27);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE28");
		lua_pushinteger(lua, GL_TEXTURE28);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE29");
		lua_pushinteger(lua, GL_TEXTURE29);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE30");
		lua_pushinteger(lua, GL_TEXTURE30);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE31");
		lua_pushinteger(lua, GL_TEXTURE31);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_TEXTURE");
		lua_pushinteger(lua, GL_ACTIVE_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ACTIVE_TEXTURE");
		lua_pushinteger(lua, GL_CLIENT_ACTIVE_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_UNITS");
		lua_pushinteger(lua, GL_MAX_TEXTURE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_MAP");
		lua_pushinteger(lua, GL_NORMAL_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REFLECTION_MAP");
		lua_pushinteger(lua, GL_REFLECTION_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_CUBE_MAP");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_CUBE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_X");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_X");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Y");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Y");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Z");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Z");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CUBE_MAP_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_CUBE_MAP_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_ALPHA");
		lua_pushinteger(lua, GL_COMPRESSED_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE_ALPHA");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_INTENSITY");
		lua_pushinteger(lua, GL_COMPRESSED_INTENSITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGB");
		lua_pushinteger(lua, GL_COMPRESSED_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSION_HINT");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSION_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSED_IMAGE_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_IMAGE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSED");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_COMPRESSED_TEXTURE_FORMATS");
		lua_pushinteger(lua, GL_NUM_COMPRESSED_TEXTURE_FORMATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_TEXTURE_FORMATS");
		lua_pushinteger(lua, GL_COMPRESSED_TEXTURE_FORMATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE");
		lua_pushinteger(lua, GL_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_COVERAGE");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_COVERAGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_ONE");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_ONE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_BUFFERS");
		lua_pushinteger(lua, GL_SAMPLE_BUFFERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES");
		lua_pushinteger(lua, GL_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_VALUE");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_INVERT");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_INVERT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_BIT");
		lua_pushinteger(lua, GL_MULTISAMPLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_MODELVIEW_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_MODELVIEW_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_PROJECTION_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_PROJECTION_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_TEXTURE_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_TEXTURE_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_COLOR_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_COLOR_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE");
		lua_pushinteger(lua, GL_COMBINE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_RGB");
		lua_pushinteger(lua, GL_COMBINE_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_ALPHA");
		lua_pushinteger(lua, GL_COMBINE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_RGB");
		lua_pushinteger(lua, GL_SOURCE0_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_RGB");
		lua_pushinteger(lua, GL_SOURCE1_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_RGB");
		lua_pushinteger(lua, GL_SOURCE2_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_ALPHA");
		lua_pushinteger(lua, GL_SOURCE0_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_ALPHA");
		lua_pushinteger(lua, GL_SOURCE1_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_ALPHA");
		lua_pushinteger(lua, GL_SOURCE2_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_RGB");
		lua_pushinteger(lua, GL_OPERAND0_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_RGB");
		lua_pushinteger(lua, GL_OPERAND1_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_RGB");
		lua_pushinteger(lua, GL_OPERAND2_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_ALPHA");
		lua_pushinteger(lua, GL_OPERAND0_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_ALPHA");
		lua_pushinteger(lua, GL_OPERAND1_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_ALPHA");
		lua_pushinteger(lua, GL_OPERAND2_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_SCALE");
		lua_pushinteger(lua, GL_RGB_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD_SIGNED");
		lua_pushinteger(lua, GL_ADD_SIGNED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERPOLATE");
		lua_pushinteger(lua, GL_INTERPOLATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUBTRACT");
		lua_pushinteger(lua, GL_SUBTRACT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT");
		lua_pushinteger(lua, GL_CONSTANT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR");
		lua_pushinteger(lua, GL_PRIMARY_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PREVIOUS");
		lua_pushinteger(lua, GL_PREVIOUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGB");
		lua_pushinteger(lua, GL_DOT3_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGBA");
		lua_pushinteger(lua, GL_DOT3_RGBA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_BORDER");
		lua_pushinteger(lua, GL_CLAMP_TO_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_multitexture");
		lua_pushinteger(lua, GL_ARB_multitexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE0_ARB");
		lua_pushinteger(lua, GL_TEXTURE0_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE1_ARB");
		lua_pushinteger(lua, GL_TEXTURE1_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE2_ARB");
		lua_pushinteger(lua, GL_TEXTURE2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE3_ARB");
		lua_pushinteger(lua, GL_TEXTURE3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE4_ARB");
		lua_pushinteger(lua, GL_TEXTURE4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE5_ARB");
		lua_pushinteger(lua, GL_TEXTURE5_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE6_ARB");
		lua_pushinteger(lua, GL_TEXTURE6_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE7_ARB");
		lua_pushinteger(lua, GL_TEXTURE7_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE8_ARB");
		lua_pushinteger(lua, GL_TEXTURE8_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE9_ARB");
		lua_pushinteger(lua, GL_TEXTURE9_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE10_ARB");
		lua_pushinteger(lua, GL_TEXTURE10_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE11_ARB");
		lua_pushinteger(lua, GL_TEXTURE11_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE12_ARB");
		lua_pushinteger(lua, GL_TEXTURE12_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE13_ARB");
		lua_pushinteger(lua, GL_TEXTURE13_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE14_ARB");
		lua_pushinteger(lua, GL_TEXTURE14_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE15_ARB");
		lua_pushinteger(lua, GL_TEXTURE15_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE16_ARB");
		lua_pushinteger(lua, GL_TEXTURE16_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE17_ARB");
		lua_pushinteger(lua, GL_TEXTURE17_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE18_ARB");
		lua_pushinteger(lua, GL_TEXTURE18_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE19_ARB");
		lua_pushinteger(lua, GL_TEXTURE19_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE20_ARB");
		lua_pushinteger(lua, GL_TEXTURE20_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE21_ARB");
		lua_pushinteger(lua, GL_TEXTURE21_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE22_ARB");
		lua_pushinteger(lua, GL_TEXTURE22_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE23_ARB");
		lua_pushinteger(lua, GL_TEXTURE23_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE24_ARB");
		lua_pushinteger(lua, GL_TEXTURE24_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE25_ARB");
		lua_pushinteger(lua, GL_TEXTURE25_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE26_ARB");
		lua_pushinteger(lua, GL_TEXTURE26_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE27_ARB");
		lua_pushinteger(lua, GL_TEXTURE27_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE28_ARB");
		lua_pushinteger(lua, GL_TEXTURE28_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE29_ARB");
		lua_pushinteger(lua, GL_TEXTURE29_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE30_ARB");
		lua_pushinteger(lua, GL_TEXTURE30_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE31_ARB");
		lua_pushinteger(lua, GL_TEXTURE31_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_TEXTURE_ARB");
		lua_pushinteger(lua, GL_ACTIVE_TEXTURE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ACTIVE_TEXTURE_ARB");
		lua_pushinteger(lua, GL_CLIENT_ACTIVE_TEXTURE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_TEXTURE_UNITS_ARB);
		lua_settable(lua, -3);
		//lua_pushstring(lua, "MESA_packed_depth_stencil");
		//lua_pushinteger(lua, GL_MESA_packed_depth_stencil);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "DEPTH_STENCIL_MESA");
		//lua_pushinteger(lua, GL_DEPTH_STENCIL_MESA);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "UNSIGNED_INT_24_8_MESA");
		//lua_pushinteger(lua, GL_UNSIGNED_INT_24_8_MESA);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "UNSIGNED_INT_8_24_REV_MESA");
		//lua_pushinteger(lua, GL_UNSIGNED_INT_8_24_REV_MESA);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "UNSIGNED_SHORT_15_1_MESA");
		//lua_pushinteger(lua, GL_UNSIGNED_SHORT_15_1_MESA);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "UNSIGNED_SHORT_1_15_REV_MESA");
		//lua_pushinteger(lua, GL_UNSIGNED_SHORT_1_15_REV_MESA);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "ATI_blend_equation_separate");
		//lua_pushinteger(lua, GL_ATI_blend_equation_separate);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "ALPHA_BLEND_EQUATION_ATI");
		//lua_pushinteger(lua, GL_ALPHA_BLEND_EQUATION_ATI);
		//lua_settable(lua, -3);
		//lua_pushstring(lua, "OES_EGL_image");
		//lua_pushinteger(lua, GL_OES_EGL_image);
		//lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_1");
		lua_pushinteger(lua, GL_VERSION_1_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZERO");
		lua_pushinteger(lua, GL_ZERO);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FALSE");
		lua_pushinteger(lua, GL_FALSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOGIC_OP");
		lua_pushinteger(lua, GL_LOGIC_OP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NONE");
		lua_pushinteger(lua, GL_NONE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPONENTS");
		lua_pushinteger(lua, GL_TEXTURE_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NO_ERROR");
		lua_pushinteger(lua, GL_NO_ERROR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINTS");
		lua_pushinteger(lua, GL_POINTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_BIT");
		lua_pushinteger(lua, GL_CURRENT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRUE");
		lua_pushinteger(lua, GL_TRUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE");
		lua_pushinteger(lua, GL_ONE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_PIXEL_STORE_BIT");
		lua_pushinteger(lua, GL_CLIENT_PIXEL_STORE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINES");
		lua_pushinteger(lua, GL_LINES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_LOOP");
		lua_pushinteger(lua, GL_LINE_LOOP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_BIT");
		lua_pushinteger(lua, GL_POINT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_VERTEX_ARRAY_BIT");
		lua_pushinteger(lua, GL_CLIENT_VERTEX_ARRAY_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STRIP");
		lua_pushinteger(lua, GL_LINE_STRIP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_BIT");
		lua_pushinteger(lua, GL_LINE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLES");
		lua_pushinteger(lua, GL_TRIANGLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_STRIP");
		lua_pushinteger(lua, GL_TRIANGLE_STRIP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_FAN");
		lua_pushinteger(lua, GL_TRIANGLE_FAN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUADS");
		lua_pushinteger(lua, GL_QUADS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUAD_STRIP");
		lua_pushinteger(lua, GL_QUAD_STRIP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_BIT");
		lua_pushinteger(lua, GL_POLYGON_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON");
		lua_pushinteger(lua, GL_POLYGON);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_STIPPLE_BIT");
		lua_pushinteger(lua, GL_POLYGON_STIPPLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MODE_BIT");
		lua_pushinteger(lua, GL_PIXEL_MODE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHTING_BIT");
		lua_pushinteger(lua, GL_LIGHTING_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_BIT");
		lua_pushinteger(lua, GL_FOG_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BUFFER_BIT");
		lua_pushinteger(lua, GL_DEPTH_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM");
		lua_pushinteger(lua, GL_ACCUM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOAD");
		lua_pushinteger(lua, GL_LOAD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RETURN");
		lua_pushinteger(lua, GL_RETURN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULT");
		lua_pushinteger(lua, GL_MULT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD");
		lua_pushinteger(lua, GL_ADD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEVER");
		lua_pushinteger(lua, GL_NEVER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_BUFFER_BIT");
		lua_pushinteger(lua, GL_ACCUM_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LESS");
		lua_pushinteger(lua, GL_LESS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EQUAL");
		lua_pushinteger(lua, GL_EQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LEQUAL");
		lua_pushinteger(lua, GL_LEQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREATER");
		lua_pushinteger(lua, GL_GREATER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NOTEQUAL");
		lua_pushinteger(lua, GL_NOTEQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEQUAL");
		lua_pushinteger(lua, GL_GEQUAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALWAYS");
		lua_pushinteger(lua, GL_ALWAYS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC_COLOR");
		lua_pushinteger(lua, GL_SRC_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_SRC_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_SRC_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC_ALPHA");
		lua_pushinteger(lua, GL_SRC_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_SRC_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_SRC_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DST_ALPHA");
		lua_pushinteger(lua, GL_DST_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_DST_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_DST_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DST_COLOR");
		lua_pushinteger(lua, GL_DST_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_DST_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_DST_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC_ALPHA_SATURATE");
		lua_pushinteger(lua, GL_SRC_ALPHA_SATURATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BUFFER_BIT");
		lua_pushinteger(lua, GL_STENCIL_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_LEFT");
		lua_pushinteger(lua, GL_FRONT_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_RIGHT");
		lua_pushinteger(lua, GL_FRONT_RIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_LEFT");
		lua_pushinteger(lua, GL_BACK_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_RIGHT");
		lua_pushinteger(lua, GL_BACK_RIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT");
		lua_pushinteger(lua, GL_FRONT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK");
		lua_pushinteger(lua, GL_BACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LEFT");
		lua_pushinteger(lua, GL_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RIGHT");
		lua_pushinteger(lua, GL_RIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_AND_BACK");
		lua_pushinteger(lua, GL_FRONT_AND_BACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX0");
		lua_pushinteger(lua, GL_AUX0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX1");
		lua_pushinteger(lua, GL_AUX1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX2");
		lua_pushinteger(lua, GL_AUX2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX3");
		lua_pushinteger(lua, GL_AUX3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_ENUM");
		lua_pushinteger(lua, GL_INVALID_ENUM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_VALUE");
		lua_pushinteger(lua, GL_INVALID_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_OPERATION");
		lua_pushinteger(lua, GL_INVALID_OPERATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STACK_OVERFLOW");
		lua_pushinteger(lua, GL_STACK_OVERFLOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STACK_UNDERFLOW");
		lua_pushinteger(lua, GL_STACK_UNDERFLOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUT_OF_MEMORY");
		lua_pushinteger(lua, GL_OUT_OF_MEMORY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2D");
		lua_pushinteger(lua, GL_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3D");
		lua_pushinteger(lua, GL_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3D_COLOR");
		lua_pushinteger(lua, GL_3D_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3D_COLOR_TEXTURE");
		lua_pushinteger(lua, GL_3D_COLOR_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4D_COLOR_TEXTURE");
		lua_pushinteger(lua, GL_4D_COLOR_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PASS_THROUGH_TOKEN");
		lua_pushinteger(lua, GL_PASS_THROUGH_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_TOKEN");
		lua_pushinteger(lua, GL_POINT_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_TOKEN");
		lua_pushinteger(lua, GL_LINE_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_TOKEN");
		lua_pushinteger(lua, GL_POLYGON_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BITMAP_TOKEN");
		lua_pushinteger(lua, GL_BITMAP_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_PIXEL_TOKEN");
		lua_pushinteger(lua, GL_DRAW_PIXEL_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY_PIXEL_TOKEN");
		lua_pushinteger(lua, GL_COPY_PIXEL_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_RESET_TOKEN");
		lua_pushinteger(lua, GL_LINE_RESET_TOKEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXP");
		lua_pushinteger(lua, GL_EXP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT_BIT");
		lua_pushinteger(lua, GL_VIEWPORT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXP2");
		lua_pushinteger(lua, GL_EXP2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CW");
		lua_pushinteger(lua, GL_CW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CCW");
		lua_pushinteger(lua, GL_CCW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COEFF");
		lua_pushinteger(lua, GL_COEFF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ORDER");
		lua_pushinteger(lua, GL_ORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOMAIN");
		lua_pushinteger(lua, GL_DOMAIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_COLOR");
		lua_pushinteger(lua, GL_CURRENT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_INDEX");
		lua_pushinteger(lua, GL_CURRENT_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_NORMAL");
		lua_pushinteger(lua, GL_CURRENT_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_TEXTURE_COORDS");
		lua_pushinteger(lua, GL_CURRENT_TEXTURE_COORDS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_COLOR");
		lua_pushinteger(lua, GL_CURRENT_RASTER_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_INDEX");
		lua_pushinteger(lua, GL_CURRENT_RASTER_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_TEXTURE_COORDS");
		lua_pushinteger(lua, GL_CURRENT_RASTER_TEXTURE_COORDS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_POSITION");
		lua_pushinteger(lua, GL_CURRENT_RASTER_POSITION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_POSITION_VALID");
		lua_pushinteger(lua, GL_CURRENT_RASTER_POSITION_VALID);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_DISTANCE");
		lua_pushinteger(lua, GL_CURRENT_RASTER_DISTANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SMOOTH");
		lua_pushinteger(lua, GL_POINT_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE");
		lua_pushinteger(lua, GL_POINT_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_RANGE");
		lua_pushinteger(lua, GL_POINT_SIZE_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_GRANULARITY");
		lua_pushinteger(lua, GL_POINT_SIZE_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_SMOOTH");
		lua_pushinteger(lua, GL_LINE_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_WIDTH");
		lua_pushinteger(lua, GL_LINE_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_WIDTH_RANGE");
		lua_pushinteger(lua, GL_LINE_WIDTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_WIDTH_GRANULARITY");
		lua_pushinteger(lua, GL_LINE_WIDTH_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STIPPLE");
		lua_pushinteger(lua, GL_LINE_STIPPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STIPPLE_PATTERN");
		lua_pushinteger(lua, GL_LINE_STIPPLE_PATTERN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STIPPLE_REPEAT");
		lua_pushinteger(lua, GL_LINE_STIPPLE_REPEAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_MODE");
		lua_pushinteger(lua, GL_LIST_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_LIST_NESTING");
		lua_pushinteger(lua, GL_MAX_LIST_NESTING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_BASE");
		lua_pushinteger(lua, GL_LIST_BASE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_INDEX");
		lua_pushinteger(lua, GL_LIST_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_MODE");
		lua_pushinteger(lua, GL_POLYGON_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_SMOOTH");
		lua_pushinteger(lua, GL_POLYGON_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_STIPPLE");
		lua_pushinteger(lua, GL_POLYGON_STIPPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG");
		lua_pushinteger(lua, GL_EDGE_FLAG);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_FACE");
		lua_pushinteger(lua, GL_CULL_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_FACE_MODE");
		lua_pushinteger(lua, GL_CULL_FACE_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRONT_FACE");
		lua_pushinteger(lua, GL_FRONT_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHTING");
		lua_pushinteger(lua, GL_LIGHTING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_LOCAL_VIEWER");
		lua_pushinteger(lua, GL_LIGHT_MODEL_LOCAL_VIEWER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_TWO_SIDE");
		lua_pushinteger(lua, GL_LIGHT_MODEL_TWO_SIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_AMBIENT");
		lua_pushinteger(lua, GL_LIGHT_MODEL_AMBIENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADE_MODEL");
		lua_pushinteger(lua, GL_SHADE_MODEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATERIAL_FACE");
		lua_pushinteger(lua, GL_COLOR_MATERIAL_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATERIAL_PARAMETER");
		lua_pushinteger(lua, GL_COLOR_MATERIAL_PARAMETER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATERIAL");
		lua_pushinteger(lua, GL_COLOR_MATERIAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG");
		lua_pushinteger(lua, GL_FOG);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_INDEX");
		lua_pushinteger(lua, GL_FOG_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_DENSITY");
		lua_pushinteger(lua, GL_FOG_DENSITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_START");
		lua_pushinteger(lua, GL_FOG_START);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_END");
		lua_pushinteger(lua, GL_FOG_END);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_MODE");
		lua_pushinteger(lua, GL_FOG_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COLOR");
		lua_pushinteger(lua, GL_FOG_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_RANGE");
		lua_pushinteger(lua, GL_DEPTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_TEST");
		lua_pushinteger(lua, GL_DEPTH_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_WRITEMASK");
		lua_pushinteger(lua, GL_DEPTH_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_CLEAR_VALUE");
		lua_pushinteger(lua, GL_DEPTH_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_FUNC");
		lua_pushinteger(lua, GL_DEPTH_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_CLEAR_VALUE");
		lua_pushinteger(lua, GL_ACCUM_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_TEST");
		lua_pushinteger(lua, GL_STENCIL_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_CLEAR_VALUE");
		lua_pushinteger(lua, GL_STENCIL_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_FUNC");
		lua_pushinteger(lua, GL_STENCIL_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_VALUE_MASK");
		lua_pushinteger(lua, GL_STENCIL_VALUE_MASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_FAIL");
		lua_pushinteger(lua, GL_STENCIL_FAIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_PASS_DEPTH_FAIL");
		lua_pushinteger(lua, GL_STENCIL_PASS_DEPTH_FAIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_PASS_DEPTH_PASS");
		lua_pushinteger(lua, GL_STENCIL_PASS_DEPTH_PASS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_REF");
		lua_pushinteger(lua, GL_STENCIL_REF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_WRITEMASK");
		lua_pushinteger(lua, GL_STENCIL_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_MODE");
		lua_pushinteger(lua, GL_MATRIX_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMALIZE");
		lua_pushinteger(lua, GL_NORMALIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT");
		lua_pushinteger(lua, GL_VIEWPORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW_STACK_DEPTH");
		lua_pushinteger(lua, GL_MODELVIEW_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROJECTION_STACK_DEPTH");
		lua_pushinteger(lua, GL_PROJECTION_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_STACK_DEPTH");
		lua_pushinteger(lua, GL_TEXTURE_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW_MATRIX");
		lua_pushinteger(lua, GL_MODELVIEW_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROJECTION_MATRIX");
		lua_pushinteger(lua, GL_PROJECTION_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MATRIX");
		lua_pushinteger(lua, GL_TEXTURE_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_CLIENT_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_TEST");
		lua_pushinteger(lua, GL_ALPHA_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_TEST_FUNC");
		lua_pushinteger(lua, GL_ALPHA_TEST_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_TEST_REF");
		lua_pushinteger(lua, GL_ALPHA_TEST_REF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DITHER");
		lua_pushinteger(lua, GL_DITHER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_DST");
		lua_pushinteger(lua, GL_BLEND_DST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_SRC");
		lua_pushinteger(lua, GL_BLEND_SRC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND");
		lua_pushinteger(lua, GL_BLEND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOGIC_OP_MODE");
		lua_pushinteger(lua, GL_LOGIC_OP_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_LOGIC_OP");
		lua_pushinteger(lua, GL_INDEX_LOGIC_OP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_LOGIC_OP");
		lua_pushinteger(lua, GL_COLOR_LOGIC_OP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX_BUFFERS");
		lua_pushinteger(lua, GL_AUX_BUFFERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER");
		lua_pushinteger(lua, GL_DRAW_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_BUFFER");
		lua_pushinteger(lua, GL_READ_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_BOX");
		lua_pushinteger(lua, GL_SCISSOR_BOX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_TEST");
		lua_pushinteger(lua, GL_SCISSOR_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_CLEAR_VALUE");
		lua_pushinteger(lua, GL_INDEX_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_WRITEMASK");
		lua_pushinteger(lua, GL_INDEX_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_CLEAR_VALUE");
		lua_pushinteger(lua, GL_COLOR_CLEAR_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_WRITEMASK");
		lua_pushinteger(lua, GL_COLOR_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_MODE");
		lua_pushinteger(lua, GL_INDEX_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_MODE");
		lua_pushinteger(lua, GL_RGBA_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLEBUFFER");
		lua_pushinteger(lua, GL_DOUBLEBUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STEREO");
		lua_pushinteger(lua, GL_STEREO);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDER_MODE");
		lua_pushinteger(lua, GL_RENDER_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERSPECTIVE_CORRECTION_HINT");
		lua_pushinteger(lua, GL_PERSPECTIVE_CORRECTION_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SMOOTH_HINT");
		lua_pushinteger(lua, GL_POINT_SMOOTH_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_SMOOTH_HINT");
		lua_pushinteger(lua, GL_LINE_SMOOTH_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_SMOOTH_HINT");
		lua_pushinteger(lua, GL_POLYGON_SMOOTH_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_HINT");
		lua_pushinteger(lua, GL_FOG_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_S");
		lua_pushinteger(lua, GL_TEXTURE_GEN_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_T");
		lua_pushinteger(lua, GL_TEXTURE_GEN_T);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_R");
		lua_pushinteger(lua, GL_TEXTURE_GEN_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_Q");
		lua_pushinteger(lua, GL_TEXTURE_GEN_Q);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_I");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_S_TO_S");
		lua_pushinteger(lua, GL_PIXEL_MAP_S_TO_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_R");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_G");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_G);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_B");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_B);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_A");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_A);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_R_TO_R");
		lua_pushinteger(lua, GL_PIXEL_MAP_R_TO_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_G_TO_G");
		lua_pushinteger(lua, GL_PIXEL_MAP_G_TO_G);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_B_TO_B");
		lua_pushinteger(lua, GL_PIXEL_MAP_B_TO_B);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_A_TO_A");
		lua_pushinteger(lua, GL_PIXEL_MAP_A_TO_A);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_I_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_I_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_S_TO_S_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_S_TO_S_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_R_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_R_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_G_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_G_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_B_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_B_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_I_TO_A_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_I_TO_A_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_R_TO_R_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_R_TO_R_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_G_TO_G_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_G_TO_G_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_B_TO_B_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_B_TO_B_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAP_A_TO_A_SIZE");
		lua_pushinteger(lua, GL_PIXEL_MAP_A_TO_A_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SWAP_BYTES");
		lua_pushinteger(lua, GL_UNPACK_SWAP_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_LSB_FIRST");
		lua_pushinteger(lua, GL_UNPACK_LSB_FIRST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_ROW_LENGTH");
		lua_pushinteger(lua, GL_UNPACK_ROW_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_ROWS");
		lua_pushinteger(lua, GL_UNPACK_SKIP_ROWS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_PIXELS");
		lua_pushinteger(lua, GL_UNPACK_SKIP_PIXELS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_ALIGNMENT");
		lua_pushinteger(lua, GL_UNPACK_ALIGNMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SWAP_BYTES");
		lua_pushinteger(lua, GL_PACK_SWAP_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_LSB_FIRST");
		lua_pushinteger(lua, GL_PACK_LSB_FIRST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_ROW_LENGTH");
		lua_pushinteger(lua, GL_PACK_ROW_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_ROWS");
		lua_pushinteger(lua, GL_PACK_SKIP_ROWS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_PIXELS");
		lua_pushinteger(lua, GL_PACK_SKIP_PIXELS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_ALIGNMENT");
		lua_pushinteger(lua, GL_PACK_ALIGNMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_COLOR");
		lua_pushinteger(lua, GL_MAP_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_STENCIL");
		lua_pushinteger(lua, GL_MAP_STENCIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_SHIFT");
		lua_pushinteger(lua, GL_INDEX_SHIFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_OFFSET");
		lua_pushinteger(lua, GL_INDEX_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_SCALE");
		lua_pushinteger(lua, GL_RED_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_BIAS");
		lua_pushinteger(lua, GL_RED_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZOOM_X");
		lua_pushinteger(lua, GL_ZOOM_X);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZOOM_Y");
		lua_pushinteger(lua, GL_ZOOM_Y);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_SCALE");
		lua_pushinteger(lua, GL_GREEN_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_BIAS");
		lua_pushinteger(lua, GL_GREEN_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_SCALE");
		lua_pushinteger(lua, GL_BLUE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_BIAS");
		lua_pushinteger(lua, GL_BLUE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_SCALE");
		lua_pushinteger(lua, GL_ALPHA_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_BIAS");
		lua_pushinteger(lua, GL_ALPHA_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_SCALE");
		lua_pushinteger(lua, GL_DEPTH_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BIAS");
		lua_pushinteger(lua, GL_DEPTH_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_EVAL_ORDER");
		lua_pushinteger(lua, GL_MAX_EVAL_ORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_LIGHTS");
		lua_pushinteger(lua, GL_MAX_LIGHTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CLIP_PLANES");
		lua_pushinteger(lua, GL_MAX_CLIP_PLANES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PIXEL_MAP_TABLE");
		lua_pushinteger(lua, GL_MAX_PIXEL_MAP_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_MODELVIEW_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_MODELVIEW_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_NAME_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_NAME_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROJECTION_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_PROJECTION_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_TEXTURE_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VIEWPORT_DIMS");
		lua_pushinteger(lua, GL_MAX_VIEWPORT_DIMS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CLIENT_ATTRIB_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUBPIXEL_BITS");
		lua_pushinteger(lua, GL_SUBPIXEL_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_BITS");
		lua_pushinteger(lua, GL_INDEX_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_BITS");
		lua_pushinteger(lua, GL_RED_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_BITS");
		lua_pushinteger(lua, GL_GREEN_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_BITS");
		lua_pushinteger(lua, GL_BLUE_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_BITS");
		lua_pushinteger(lua, GL_ALPHA_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BITS");
		lua_pushinteger(lua, GL_DEPTH_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BITS");
		lua_pushinteger(lua, GL_STENCIL_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_RED_BITS");
		lua_pushinteger(lua, GL_ACCUM_RED_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_GREEN_BITS");
		lua_pushinteger(lua, GL_ACCUM_GREEN_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_BLUE_BITS");
		lua_pushinteger(lua, GL_ACCUM_BLUE_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACCUM_ALPHA_BITS");
		lua_pushinteger(lua, GL_ACCUM_ALPHA_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NAME_STACK_DEPTH");
		lua_pushinteger(lua, GL_NAME_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUTO_NORMAL");
		lua_pushinteger(lua, GL_AUTO_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_COLOR_4");
		lua_pushinteger(lua, GL_MAP1_COLOR_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_INDEX");
		lua_pushinteger(lua, GL_MAP1_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_NORMAL");
		lua_pushinteger(lua, GL_MAP1_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_1");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_2");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_3");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TEXTURE_COORD_4");
		lua_pushinteger(lua, GL_MAP1_TEXTURE_COORD_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_3");
		lua_pushinteger(lua, GL_MAP1_VERTEX_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_4");
		lua_pushinteger(lua, GL_MAP1_VERTEX_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_COLOR_4");
		lua_pushinteger(lua, GL_MAP2_COLOR_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_INDEX");
		lua_pushinteger(lua, GL_MAP2_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_NORMAL");
		lua_pushinteger(lua, GL_MAP2_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_1");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_2");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_3");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TEXTURE_COORD_4");
		lua_pushinteger(lua, GL_MAP2_TEXTURE_COORD_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_3");
		lua_pushinteger(lua, GL_MAP2_VERTEX_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_4");
		lua_pushinteger(lua, GL_MAP2_VERTEX_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_GRID_DOMAIN");
		lua_pushinteger(lua, GL_MAP1_GRID_DOMAIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_GRID_SEGMENTS");
		lua_pushinteger(lua, GL_MAP1_GRID_SEGMENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_GRID_DOMAIN");
		lua_pushinteger(lua, GL_MAP2_GRID_DOMAIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_GRID_SEGMENTS");
		lua_pushinteger(lua, GL_MAP2_GRID_SEGMENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D");
		lua_pushinteger(lua, GL_TEXTURE_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D");
		lua_pushinteger(lua, GL_TEXTURE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK_BUFFER_POINTER");
		lua_pushinteger(lua, GL_FEEDBACK_BUFFER_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK_BUFFER_SIZE");
		lua_pushinteger(lua, GL_FEEDBACK_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK_BUFFER_TYPE");
		lua_pushinteger(lua, GL_FEEDBACK_BUFFER_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SELECTION_BUFFER_POINTER");
		lua_pushinteger(lua, GL_SELECTION_BUFFER_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SELECTION_BUFFER_SIZE");
		lua_pushinteger(lua, GL_SELECTION_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WIDTH");
		lua_pushinteger(lua, GL_TEXTURE_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_BIT");
		lua_pushinteger(lua, GL_TRANSFORM_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_HEIGHT");
		lua_pushinteger(lua, GL_TEXTURE_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTERNAL_FORMAT");
		lua_pushinteger(lua, GL_TEXTURE_INTERNAL_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BORDER_COLOR");
		lua_pushinteger(lua, GL_TEXTURE_BORDER_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BORDER");
		lua_pushinteger(lua, GL_TEXTURE_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DONT_CARE");
		lua_pushinteger(lua, GL_DONT_CARE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FASTEST");
		lua_pushinteger(lua, GL_FASTEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NICEST");
		lua_pushinteger(lua, GL_NICEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AMBIENT");
		lua_pushinteger(lua, GL_AMBIENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DIFFUSE");
		lua_pushinteger(lua, GL_DIFFUSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPECULAR");
		lua_pushinteger(lua, GL_SPECULAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POSITION");
		lua_pushinteger(lua, GL_POSITION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPOT_DIRECTION");
		lua_pushinteger(lua, GL_SPOT_DIRECTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPOT_EXPONENT");
		lua_pushinteger(lua, GL_SPOT_EXPONENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPOT_CUTOFF");
		lua_pushinteger(lua, GL_SPOT_CUTOFF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_ATTENUATION");
		lua_pushinteger(lua, GL_CONSTANT_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR_ATTENUATION");
		lua_pushinteger(lua, GL_LINEAR_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUADRATIC_ATTENUATION");
		lua_pushinteger(lua, GL_QUADRATIC_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPILE");
		lua_pushinteger(lua, GL_COMPILE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPILE_AND_EXECUTE");
		lua_pushinteger(lua, GL_COMPILE_AND_EXECUTE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BYTE");
		lua_pushinteger(lua, GL_BYTE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHORT");
		lua_pushinteger(lua, GL_SHORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT");
		lua_pushinteger(lua, GL_INT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT");
		lua_pushinteger(lua, GL_UNSIGNED_INT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT");
		lua_pushinteger(lua, GL_FLOAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2_BYTES");
		lua_pushinteger(lua, GL_2_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3_BYTES");
		lua_pushinteger(lua, GL_3_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4_BYTES");
		lua_pushinteger(lua, GL_4_BYTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE");
		lua_pushinteger(lua, GL_DOUBLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLEAR");
		lua_pushinteger(lua, GL_CLEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AND");
		lua_pushinteger(lua, GL_AND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AND_REVERSE");
		lua_pushinteger(lua, GL_AND_REVERSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY");
		lua_pushinteger(lua, GL_COPY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AND_INVERTED");
		lua_pushinteger(lua, GL_AND_INVERTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NOOP");
		lua_pushinteger(lua, GL_NOOP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "XOR");
		lua_pushinteger(lua, GL_XOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OR");
		lua_pushinteger(lua, GL_OR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NOR");
		lua_pushinteger(lua, GL_NOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EQUIV");
		lua_pushinteger(lua, GL_EQUIV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVERT");
		lua_pushinteger(lua, GL_INVERT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OR_REVERSE");
		lua_pushinteger(lua, GL_OR_REVERSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY_INVERTED");
		lua_pushinteger(lua, GL_COPY_INVERTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OR_INVERTED");
		lua_pushinteger(lua, GL_OR_INVERTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NAND");
		lua_pushinteger(lua, GL_NAND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SET");
		lua_pushinteger(lua, GL_SET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EMISSION");
		lua_pushinteger(lua, GL_EMISSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHININESS");
		lua_pushinteger(lua, GL_SHININESS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AMBIENT_AND_DIFFUSE");
		lua_pushinteger(lua, GL_AMBIENT_AND_DIFFUSE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEXES");
		lua_pushinteger(lua, GL_COLOR_INDEXES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW");
		lua_pushinteger(lua, GL_MODELVIEW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROJECTION");
		lua_pushinteger(lua, GL_PROJECTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE");
		lua_pushinteger(lua, GL_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR");
		lua_pushinteger(lua, GL_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH");
		lua_pushinteger(lua, GL_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL");
		lua_pushinteger(lua, GL_STENCIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX");
		lua_pushinteger(lua, GL_COLOR_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX");
		lua_pushinteger(lua, GL_STENCIL_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED");
		lua_pushinteger(lua, GL_RED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN");
		lua_pushinteger(lua, GL_GREEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE");
		lua_pushinteger(lua, GL_BLUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA");
		lua_pushinteger(lua, GL_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB");
		lua_pushinteger(lua, GL_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA");
		lua_pushinteger(lua, GL_RGBA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE");
		lua_pushinteger(lua, GL_LUMINANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BITMAP");
		lua_pushinteger(lua, GL_BITMAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT");
		lua_pushinteger(lua, GL_POINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE");
		lua_pushinteger(lua, GL_LINE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FILL");
		lua_pushinteger(lua, GL_FILL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDER");
		lua_pushinteger(lua, GL_RENDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FEEDBACK");
		lua_pushinteger(lua, GL_FEEDBACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SELECT");
		lua_pushinteger(lua, GL_SELECT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLAT");
		lua_pushinteger(lua, GL_FLAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH");
		lua_pushinteger(lua, GL_SMOOTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "KEEP");
		lua_pushinteger(lua, GL_KEEP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACE");
		lua_pushinteger(lua, GL_REPLACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INCR");
		lua_pushinteger(lua, GL_INCR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DECR");
		lua_pushinteger(lua, GL_DECR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VENDOR");
		lua_pushinteger(lua, GL_VENDOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERER");
		lua_pushinteger(lua, GL_RENDERER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION");
		lua_pushinteger(lua, GL_VERSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXTENSIONS");
		lua_pushinteger(lua, GL_EXTENSIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "S");
		lua_pushinteger(lua, GL_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ENABLE_BIT");
		lua_pushinteger(lua, GL_ENABLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T");
		lua_pushinteger(lua, GL_T);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R");
		lua_pushinteger(lua, GL_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Q");
		lua_pushinteger(lua, GL_Q);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODULATE");
		lua_pushinteger(lua, GL_MODULATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DECAL");
		lua_pushinteger(lua, GL_DECAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ENV_MODE");
		lua_pushinteger(lua, GL_TEXTURE_ENV_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ENV_COLOR");
		lua_pushinteger(lua, GL_TEXTURE_ENV_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ENV");
		lua_pushinteger(lua, GL_TEXTURE_ENV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_LINEAR");
		lua_pushinteger(lua, GL_EYE_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_LINEAR");
		lua_pushinteger(lua, GL_OBJECT_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPHERE_MAP");
		lua_pushinteger(lua, GL_SPHERE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEN_MODE");
		lua_pushinteger(lua, GL_TEXTURE_GEN_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_PLANE");
		lua_pushinteger(lua, GL_OBJECT_PLANE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_PLANE");
		lua_pushinteger(lua, GL_EYE_PLANE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEAREST");
		lua_pushinteger(lua, GL_NEAREST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR");
		lua_pushinteger(lua, GL_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEAREST_MIPMAP_NEAREST");
		lua_pushinteger(lua, GL_NEAREST_MIPMAP_NEAREST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR_MIPMAP_NEAREST");
		lua_pushinteger(lua, GL_LINEAR_MIPMAP_NEAREST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEAREST_MIPMAP_LINEAR");
		lua_pushinteger(lua, GL_NEAREST_MIPMAP_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINEAR_MIPMAP_LINEAR");
		lua_pushinteger(lua, GL_LINEAR_MIPMAP_LINEAR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAG_FILTER");
		lua_pushinteger(lua, GL_TEXTURE_MAG_FILTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MIN_FILTER");
		lua_pushinteger(lua, GL_TEXTURE_MIN_FILTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_S");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_S);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_T");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_T);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP");
		lua_pushinteger(lua, GL_CLAMP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPEAT");
		lua_pushinteger(lua, GL_REPEAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_UNITS");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_POINT");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_POINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_LINE");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_LINE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R3_G3_B2");
		lua_pushinteger(lua, GL_R3_G3_B2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "V2F");
		lua_pushinteger(lua, GL_V2F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "V3F");
		lua_pushinteger(lua, GL_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C4UB_V2F");
		lua_pushinteger(lua, GL_C4UB_V2F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C4UB_V3F");
		lua_pushinteger(lua, GL_C4UB_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C3F_V3F");
		lua_pushinteger(lua, GL_C3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "N3F_V3F");
		lua_pushinteger(lua, GL_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "C4F_N3F_V3F");
		lua_pushinteger(lua, GL_C4F_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_V3F");
		lua_pushinteger(lua, GL_T2F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T4F_V4F");
		lua_pushinteger(lua, GL_T4F_V4F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_C4UB_V3F");
		lua_pushinteger(lua, GL_T2F_C4UB_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_C3F_V3F");
		lua_pushinteger(lua, GL_T2F_C3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_N3F_V3F");
		lua_pushinteger(lua, GL_T2F_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T2F_C4F_N3F_V3F");
		lua_pushinteger(lua, GL_T2F_C4F_N3F_V3F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "T4F_C4F_N3F_V4F");
		lua_pushinteger(lua, GL_T4F_C4F_N3F_V4F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE0");
		lua_pushinteger(lua, GL_CLIP_PLANE0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE1");
		lua_pushinteger(lua, GL_CLIP_PLANE1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE2");
		lua_pushinteger(lua, GL_CLIP_PLANE2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE3");
		lua_pushinteger(lua, GL_CLIP_PLANE3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE4");
		lua_pushinteger(lua, GL_CLIP_PLANE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_PLANE5");
		lua_pushinteger(lua, GL_CLIP_PLANE5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT0");
		lua_pushinteger(lua, GL_LIGHT0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_BUFFER_BIT");
		lua_pushinteger(lua, GL_COLOR_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT1");
		lua_pushinteger(lua, GL_LIGHT1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT2");
		lua_pushinteger(lua, GL_LIGHT2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT3");
		lua_pushinteger(lua, GL_LIGHT3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT4");
		lua_pushinteger(lua, GL_LIGHT4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT5");
		lua_pushinteger(lua, GL_LIGHT5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT6");
		lua_pushinteger(lua, GL_LIGHT6);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT7");
		lua_pushinteger(lua, GL_LIGHT7);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HINT_BIT");
		lua_pushinteger(lua, GL_HINT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_FILL");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_FILL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_FACTOR");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_FACTOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA4");
		lua_pushinteger(lua, GL_ALPHA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA8");
		lua_pushinteger(lua, GL_ALPHA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA12");
		lua_pushinteger(lua, GL_ALPHA12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16");
		lua_pushinteger(lua, GL_ALPHA16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE4");
		lua_pushinteger(lua, GL_LUMINANCE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8");
		lua_pushinteger(lua, GL_LUMINANCE8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12");
		lua_pushinteger(lua, GL_LUMINANCE12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16");
		lua_pushinteger(lua, GL_LUMINANCE16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE4_ALPHA4");
		lua_pushinteger(lua, GL_LUMINANCE4_ALPHA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE6_ALPHA2");
		lua_pushinteger(lua, GL_LUMINANCE6_ALPHA2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8_ALPHA8");
		lua_pushinteger(lua, GL_LUMINANCE8_ALPHA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_ALPHA4");
		lua_pushinteger(lua, GL_LUMINANCE12_ALPHA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_ALPHA12");
		lua_pushinteger(lua, GL_LUMINANCE12_ALPHA12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16_ALPHA16");
		lua_pushinteger(lua, GL_LUMINANCE16_ALPHA16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY");
		lua_pushinteger(lua, GL_INTENSITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY4");
		lua_pushinteger(lua, GL_INTENSITY4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY8");
		lua_pushinteger(lua, GL_INTENSITY8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY12");
		lua_pushinteger(lua, GL_INTENSITY12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16");
		lua_pushinteger(lua, GL_INTENSITY16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB4");
		lua_pushinteger(lua, GL_RGB4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB5");
		lua_pushinteger(lua, GL_RGB5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8");
		lua_pushinteger(lua, GL_RGB8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10");
		lua_pushinteger(lua, GL_RGB10);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB12");
		lua_pushinteger(lua, GL_RGB12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16");
		lua_pushinteger(lua, GL_RGB16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA2");
		lua_pushinteger(lua, GL_RGBA2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA4");
		lua_pushinteger(lua, GL_RGBA4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB5_A1");
		lua_pushinteger(lua, GL_RGB5_A1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8");
		lua_pushinteger(lua, GL_RGBA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10_A2");
		lua_pushinteger(lua, GL_RGB10_A2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA12");
		lua_pushinteger(lua, GL_RGBA12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16");
		lua_pushinteger(lua, GL_RGBA16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RED_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GREEN_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BLUE_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ALPHA_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LUMINANCE_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_LUMINANCE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTENSITY_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_INTENSITY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_PRIORITY");
		lua_pushinteger(lua, GL_TEXTURE_PRIORITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RESIDENT");
		lua_pushinteger(lua, GL_TEXTURE_RESIDENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_1D");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_2D");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY");
		lua_pushinteger(lua, GL_VERTEX_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY");
		lua_pushinteger(lua, GL_NORMAL_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY");
		lua_pushinteger(lua, GL_COLOR_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY");
		lua_pushinteger(lua, GL_INDEX_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_SIZE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_TYPE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_TYPE");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_SIZE");
		lua_pushinteger(lua, GL_COLOR_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_TYPE");
		lua_pushinteger(lua, GL_COLOR_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_COLOR_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_TYPE");
		lua_pushinteger(lua, GL_INDEX_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_INDEX_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_POINTER");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_POINTER");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_POINTER");
		lua_pushinteger(lua, GL_COLOR_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_POINTER");
		lua_pushinteger(lua, GL_INDEX_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_POINTER");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_POINTER");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX1_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX2_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX4_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX8_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX12_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX16_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_BIT");
		lua_pushinteger(lua, GL_EVAL_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIST_BIT");
		lua_pushinteger(lua, GL_LIST_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BIT");
		lua_pushinteger(lua, GL_TEXTURE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_BIT");
		lua_pushinteger(lua, GL_SCISSOR_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALL_ATTRIB_BITS");
		lua_pushinteger(lua, GL_ALL_ATTRIB_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ALL_ATTRIB_BITS");
		lua_pushinteger(lua, GL_CLIENT_ALL_ATTRIB_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_2");
		lua_pushinteger(lua, GL_VERSION_1_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_POINT_SIZE_RANGE");
		lua_pushinteger(lua, GL_SMOOTH_POINT_SIZE_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_POINT_SIZE_GRANULARITY");
		lua_pushinteger(lua, GL_SMOOTH_POINT_SIZE_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_LINE_WIDTH_RANGE");
		lua_pushinteger(lua, GL_SMOOTH_LINE_WIDTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SMOOTH_LINE_WIDTH_GRANULARITY");
		lua_pushinteger(lua, GL_SMOOTH_LINE_WIDTH_GRANULARITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE_3_3_2");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE_3_3_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_4_4_4_4");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_4_4_4_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_5_5_1");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_5_5_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_8_8");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_8_8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_10_10_10_2");
		lua_pushinteger(lua, GL_UNSIGNED_INT_10_10_10_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESCALE_NORMAL");
		lua_pushinteger(lua, GL_RESCALE_NORMAL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_3D");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_IMAGES");
		lua_pushinteger(lua, GL_PACK_SKIP_IMAGES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_IMAGE_HEIGHT");
		lua_pushinteger(lua, GL_PACK_IMAGE_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_IMAGES");
		lua_pushinteger(lua, GL_UNPACK_SKIP_IMAGES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_IMAGE_HEIGHT");
		lua_pushinteger(lua, GL_UNPACK_IMAGE_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_3D");
		lua_pushinteger(lua, GL_TEXTURE_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_3D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_R");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_3D_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_3D_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGR");
		lua_pushinteger(lua, GL_BGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA");
		lua_pushinteger(lua, GL_BGRA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ELEMENTS_VERTICES");
		lua_pushinteger(lua, GL_MAX_ELEMENTS_VERTICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ELEMENTS_INDICES");
		lua_pushinteger(lua, GL_MAX_ELEMENTS_INDICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_EDGE");
		lua_pushinteger(lua, GL_CLAMP_TO_EDGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MIN_LOD");
		lua_pushinteger(lua, GL_TEXTURE_MIN_LOD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_LOD");
		lua_pushinteger(lua, GL_TEXTURE_MAX_LOD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BASE_LEVEL");
		lua_pushinteger(lua, GL_TEXTURE_BASE_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_LEVEL");
		lua_pushinteger(lua, GL_TEXTURE_MAX_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_COLOR_CONTROL");
		lua_pushinteger(lua, GL_LIGHT_MODEL_COLOR_CONTROL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SINGLE_COLOR");
		lua_pushinteger(lua, GL_SINGLE_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARATE_SPECULAR_COLOR");
		lua_pushinteger(lua, GL_SEPARATE_SPECULAR_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE_2_3_3_REV");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE_2_3_3_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_6_5");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_6_5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_6_5_REV");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_6_5_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_4_4_4_4_REV");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_4_4_4_4_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_1_5_5_5_REV");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_1_5_5_5_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_8_8_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_8_8_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALIASED_POINT_SIZE_RANGE");
		lua_pushinteger(lua, GL_ALIASED_POINT_SIZE_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALIASED_LINE_WIDTH_RANGE");
		lua_pushinteger(lua, GL_ALIASED_LINE_WIDTH_RANGE);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "VERSION_1_2_1");
		// lua_pushinteger(lua, GL_VERSION_1_2_1);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_3");
		lua_pushinteger(lua, GL_VERSION_1_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE");
		lua_pushinteger(lua, GL_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_COVERAGE");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_COVERAGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_ONE");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_ONE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_BUFFERS");
		lua_pushinteger(lua, GL_SAMPLE_BUFFERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES");
		lua_pushinteger(lua, GL_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_VALUE");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_INVERT");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_INVERT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_BORDER");
		lua_pushinteger(lua, GL_CLAMP_TO_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE0");
		lua_pushinteger(lua, GL_TEXTURE0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE1");
		lua_pushinteger(lua, GL_TEXTURE1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE2");
		lua_pushinteger(lua, GL_TEXTURE2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE3");
		lua_pushinteger(lua, GL_TEXTURE3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE4");
		lua_pushinteger(lua, GL_TEXTURE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE5");
		lua_pushinteger(lua, GL_TEXTURE5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE6");
		lua_pushinteger(lua, GL_TEXTURE6);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE7");
		lua_pushinteger(lua, GL_TEXTURE7);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE8");
		lua_pushinteger(lua, GL_TEXTURE8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE9");
		lua_pushinteger(lua, GL_TEXTURE9);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE10");
		lua_pushinteger(lua, GL_TEXTURE10);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE11");
		lua_pushinteger(lua, GL_TEXTURE11);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE12");
		lua_pushinteger(lua, GL_TEXTURE12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE13");
		lua_pushinteger(lua, GL_TEXTURE13);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE14");
		lua_pushinteger(lua, GL_TEXTURE14);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE15");
		lua_pushinteger(lua, GL_TEXTURE15);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE16");
		lua_pushinteger(lua, GL_TEXTURE16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE17");
		lua_pushinteger(lua, GL_TEXTURE17);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE18");
		lua_pushinteger(lua, GL_TEXTURE18);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE19");
		lua_pushinteger(lua, GL_TEXTURE19);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE20");
		lua_pushinteger(lua, GL_TEXTURE20);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE21");
		lua_pushinteger(lua, GL_TEXTURE21);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE22");
		lua_pushinteger(lua, GL_TEXTURE22);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE23");
		lua_pushinteger(lua, GL_TEXTURE23);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE24");
		lua_pushinteger(lua, GL_TEXTURE24);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE25");
		lua_pushinteger(lua, GL_TEXTURE25);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE26");
		lua_pushinteger(lua, GL_TEXTURE26);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE27");
		lua_pushinteger(lua, GL_TEXTURE27);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE28");
		lua_pushinteger(lua, GL_TEXTURE28);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE29");
		lua_pushinteger(lua, GL_TEXTURE29);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE30");
		lua_pushinteger(lua, GL_TEXTURE30);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE31");
		lua_pushinteger(lua, GL_TEXTURE31);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_TEXTURE");
		lua_pushinteger(lua, GL_ACTIVE_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ACTIVE_TEXTURE");
		lua_pushinteger(lua, GL_CLIENT_ACTIVE_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_UNITS");
		lua_pushinteger(lua, GL_MAX_TEXTURE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_MODELVIEW_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_MODELVIEW_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_PROJECTION_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_PROJECTION_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_TEXTURE_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_TEXTURE_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_COLOR_MATRIX");
		lua_pushinteger(lua, GL_TRANSPOSE_COLOR_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUBTRACT");
		lua_pushinteger(lua, GL_SUBTRACT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_ALPHA");
		lua_pushinteger(lua, GL_COMPRESSED_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE_ALPHA");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_INTENSITY");
		lua_pushinteger(lua, GL_COMPRESSED_INTENSITY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGB");
		lua_pushinteger(lua, GL_COMPRESSED_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSION_HINT");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSION_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_MAP");
		lua_pushinteger(lua, GL_NORMAL_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REFLECTION_MAP");
		lua_pushinteger(lua, GL_REFLECTION_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_CUBE_MAP");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_CUBE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_X");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_X");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Y");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Y");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Z");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Z");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CUBE_MAP_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_CUBE_MAP_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE");
		lua_pushinteger(lua, GL_COMBINE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_RGB");
		lua_pushinteger(lua, GL_COMBINE_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_ALPHA");
		lua_pushinteger(lua, GL_COMBINE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_SCALE");
		lua_pushinteger(lua, GL_RGB_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD_SIGNED");
		lua_pushinteger(lua, GL_ADD_SIGNED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERPOLATE");
		lua_pushinteger(lua, GL_INTERPOLATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT");
		lua_pushinteger(lua, GL_CONSTANT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR");
		lua_pushinteger(lua, GL_PRIMARY_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PREVIOUS");
		lua_pushinteger(lua, GL_PREVIOUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_RGB");
		lua_pushinteger(lua, GL_SOURCE0_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_RGB");
		lua_pushinteger(lua, GL_SOURCE1_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_RGB");
		lua_pushinteger(lua, GL_SOURCE2_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_ALPHA");
		lua_pushinteger(lua, GL_SOURCE0_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_ALPHA");
		lua_pushinteger(lua, GL_SOURCE1_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_ALPHA");
		lua_pushinteger(lua, GL_SOURCE2_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_RGB");
		lua_pushinteger(lua, GL_OPERAND0_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_RGB");
		lua_pushinteger(lua, GL_OPERAND1_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_RGB");
		lua_pushinteger(lua, GL_OPERAND2_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_ALPHA");
		lua_pushinteger(lua, GL_OPERAND0_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_ALPHA");
		lua_pushinteger(lua, GL_OPERAND1_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_ALPHA");
		lua_pushinteger(lua, GL_OPERAND2_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSED_IMAGE_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_IMAGE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSED");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_COMPRESSED_TEXTURE_FORMATS");
		lua_pushinteger(lua, GL_NUM_COMPRESSED_TEXTURE_FORMATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_TEXTURE_FORMATS");
		lua_pushinteger(lua, GL_COMPRESSED_TEXTURE_FORMATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGB");
		lua_pushinteger(lua, GL_DOT3_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGBA");
		lua_pushinteger(lua, GL_DOT3_RGBA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_BIT");
		lua_pushinteger(lua, GL_MULTISAMPLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_4");
		lua_pushinteger(lua, GL_VERSION_1_4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_DST_RGB");
		lua_pushinteger(lua, GL_BLEND_DST_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_SRC_RGB");
		lua_pushinteger(lua, GL_BLEND_SRC_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_DST_ALPHA");
		lua_pushinteger(lua, GL_BLEND_DST_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_SRC_ALPHA");
		lua_pushinteger(lua, GL_BLEND_SRC_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_MIN");
		lua_pushinteger(lua, GL_POINT_SIZE_MIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_MAX");
		lua_pushinteger(lua, GL_POINT_SIZE_MAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_FADE_THRESHOLD_SIZE");
		lua_pushinteger(lua, GL_POINT_FADE_THRESHOLD_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_DISTANCE_ATTENUATION");
		lua_pushinteger(lua, GL_POINT_DISTANCE_ATTENUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GENERATE_MIPMAP");
		lua_pushinteger(lua, GL_GENERATE_MIPMAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GENERATE_MIPMAP_HINT");
		lua_pushinteger(lua, GL_GENERATE_MIPMAP_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT16");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT24");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT24);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT32");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT32);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRRORED_REPEAT");
		lua_pushinteger(lua, GL_MIRRORED_REPEAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_SOURCE");
		lua_pushinteger(lua, GL_FOG_COORDINATE_SOURCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE");
		lua_pushinteger(lua, GL_FOG_COORDINATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_DEPTH");
		lua_pushinteger(lua, GL_FRAGMENT_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_FOG_COORDINATE");
		lua_pushinteger(lua, GL_CURRENT_FOG_COORDINATE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_TYPE");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_POINTER");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SUM");
		lua_pushinteger(lua, GL_COLOR_SUM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_SECONDARY_COLOR");
		lua_pushinteger(lua, GL_CURRENT_SECONDARY_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_SIZE");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_TYPE");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_POINTER");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_LOD_BIAS");
		lua_pushinteger(lua, GL_MAX_TEXTURE_LOD_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_FILTER_CONTROL");
		lua_pushinteger(lua, GL_TEXTURE_FILTER_CONTROL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LOD_BIAS");
		lua_pushinteger(lua, GL_TEXTURE_LOD_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INCR_WRAP");
		lua_pushinteger(lua, GL_INCR_WRAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DECR_WRAP");
		lua_pushinteger(lua, GL_DECR_WRAP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_TEXTURE_MODE");
		lua_pushinteger(lua, GL_DEPTH_TEXTURE_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_MODE");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_FUNC");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPARE_R_TO_TEXTURE");
		lua_pushinteger(lua, GL_COMPARE_R_TO_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_1_5");
		lua_pushinteger(lua, GL_VERSION_1_5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_FOG_COORD");
		lua_pushinteger(lua, GL_CURRENT_FOG_COORD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD");
		lua_pushinteger(lua, GL_FOG_COORD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY_POINTER");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY_TYPE");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_SRC");
		lua_pushinteger(lua, GL_FOG_COORD_SRC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC0_ALPHA");
		lua_pushinteger(lua, GL_SRC0_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC0_RGB");
		lua_pushinteger(lua, GL_SRC0_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC1_ALPHA");
		lua_pushinteger(lua, GL_SRC1_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC1_RGB");
		lua_pushinteger(lua, GL_SRC1_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC2_ALPHA");
		lua_pushinteger(lua, GL_SRC2_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC2_RGB");
		lua_pushinteger(lua, GL_SRC2_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_SIZE");
		lua_pushinteger(lua, GL_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_USAGE");
		lua_pushinteger(lua, GL_BUFFER_USAGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_COUNTER_BITS");
		lua_pushinteger(lua, GL_QUERY_COUNTER_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_QUERY");
		lua_pushinteger(lua, GL_CURRENT_QUERY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_RESULT");
		lua_pushinteger(lua, GL_QUERY_RESULT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_RESULT_AVAILABLE");
		lua_pushinteger(lua, GL_QUERY_RESULT_AVAILABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_BUFFER");
		lua_pushinteger(lua, GL_ARRAY_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_BUFFER");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_COLOR_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_INDEX_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_ONLY");
		lua_pushinteger(lua, GL_READ_ONLY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRITE_ONLY");
		lua_pushinteger(lua, GL_WRITE_ONLY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_WRITE");
		lua_pushinteger(lua, GL_READ_WRITE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_ACCESS");
		lua_pushinteger(lua, GL_BUFFER_ACCESS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_MAPPED");
		lua_pushinteger(lua, GL_BUFFER_MAPPED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_MAP_POINTER");
		lua_pushinteger(lua, GL_BUFFER_MAP_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STREAM_DRAW");
		lua_pushinteger(lua, GL_STREAM_DRAW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STREAM_READ");
		lua_pushinteger(lua, GL_STREAM_READ);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STREAM_COPY");
		lua_pushinteger(lua, GL_STREAM_COPY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_DRAW");
		lua_pushinteger(lua, GL_STATIC_DRAW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_READ");
		lua_pushinteger(lua, GL_STATIC_READ);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_COPY");
		lua_pushinteger(lua, GL_STATIC_COPY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_DRAW");
		lua_pushinteger(lua, GL_DYNAMIC_DRAW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_READ");
		lua_pushinteger(lua, GL_DYNAMIC_READ);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_COPY");
		lua_pushinteger(lua, GL_DYNAMIC_COPY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES_PASSED");
		lua_pushinteger(lua, GL_SAMPLES_PASSED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_2_0");
		lua_pushinteger(lua, GL_VERSION_2_0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION_RGB");
		lua_pushinteger(lua, GL_BLEND_EQUATION_RGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_ENABLED");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_ENABLED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_SIZE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_TYPE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_VERTEX_ATTRIB");
		lua_pushinteger(lua, GL_CURRENT_VERTEX_ATTRIB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_POINT_SIZE");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_POINT_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_TWO_SIDE");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_TWO_SIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_POINTER");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_POINTER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_FUNC");
		lua_pushinteger(lua, GL_STENCIL_BACK_FUNC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_FAIL");
		lua_pushinteger(lua, GL_STENCIL_BACK_FAIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_PASS_DEPTH_FAIL");
		lua_pushinteger(lua, GL_STENCIL_BACK_PASS_DEPTH_FAIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_PASS_DEPTH_PASS");
		lua_pushinteger(lua, GL_STENCIL_BACK_PASS_DEPTH_PASS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DRAW_BUFFERS");
		lua_pushinteger(lua, GL_MAX_DRAW_BUFFERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER0");
		lua_pushinteger(lua, GL_DRAW_BUFFER0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER1");
		lua_pushinteger(lua, GL_DRAW_BUFFER1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER2");
		lua_pushinteger(lua, GL_DRAW_BUFFER2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER3");
		lua_pushinteger(lua, GL_DRAW_BUFFER3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER4");
		lua_pushinteger(lua, GL_DRAW_BUFFER4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER5");
		lua_pushinteger(lua, GL_DRAW_BUFFER5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER6");
		lua_pushinteger(lua, GL_DRAW_BUFFER6);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER7");
		lua_pushinteger(lua, GL_DRAW_BUFFER7);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER8");
		lua_pushinteger(lua, GL_DRAW_BUFFER8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER9");
		lua_pushinteger(lua, GL_DRAW_BUFFER9);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER10");
		lua_pushinteger(lua, GL_DRAW_BUFFER10);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER11");
		lua_pushinteger(lua, GL_DRAW_BUFFER11);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER12");
		lua_pushinteger(lua, GL_DRAW_BUFFER12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER13");
		lua_pushinteger(lua, GL_DRAW_BUFFER13);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER14");
		lua_pushinteger(lua, GL_DRAW_BUFFER14);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER15");
		lua_pushinteger(lua, GL_DRAW_BUFFER15);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION_ALPHA");
		lua_pushinteger(lua, GL_BLEND_EQUATION_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SPRITE");
		lua_pushinteger(lua, GL_POINT_SPRITE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COORD_REPLACE");
		lua_pushinteger(lua, GL_COORD_REPLACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_ATTRIBS");
		lua_pushinteger(lua, GL_MAX_VERTEX_ATTRIBS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_NORMALIZED");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_COORDS");
		lua_pushinteger(lua, GL_MAX_TEXTURE_COORDS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_IMAGE_UNITS");
		lua_pushinteger(lua, GL_MAX_TEXTURE_IMAGE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_SHADER");
		lua_pushinteger(lua, GL_FRAGMENT_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER");
		lua_pushinteger(lua, GL_VERTEX_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_VERTEX_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VARYING_FLOATS");
		lua_pushinteger(lua, GL_MAX_VARYING_FLOATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_TEXTURE_IMAGE_UNITS");
		lua_pushinteger(lua, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_TEXTURE_IMAGE_UNITS");
		lua_pushinteger(lua, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_TYPE");
		lua_pushinteger(lua, GL_SHADER_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_VEC2");
		lua_pushinteger(lua, GL_FLOAT_VEC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_VEC3");
		lua_pushinteger(lua, GL_FLOAT_VEC3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_VEC4");
		lua_pushinteger(lua, GL_FLOAT_VEC4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_VEC2");
		lua_pushinteger(lua, GL_INT_VEC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_VEC3");
		lua_pushinteger(lua, GL_INT_VEC3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_VEC4");
		lua_pushinteger(lua, GL_INT_VEC4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL");
		lua_pushinteger(lua, GL_BOOL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_VEC2");
		lua_pushinteger(lua, GL_BOOL_VEC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_VEC3");
		lua_pushinteger(lua, GL_BOOL_VEC3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_VEC4");
		lua_pushinteger(lua, GL_BOOL_VEC4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT2");
		lua_pushinteger(lua, GL_FLOAT_MAT2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT3");
		lua_pushinteger(lua, GL_FLOAT_MAT3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT4");
		lua_pushinteger(lua, GL_FLOAT_MAT4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D");
		lua_pushinteger(lua, GL_SAMPLER_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D");
		lua_pushinteger(lua, GL_SAMPLER_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_3D");
		lua_pushinteger(lua, GL_SAMPLER_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE");
		lua_pushinteger(lua, GL_SAMPLER_CUBE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_1D_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_2D_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DELETE_STATUS");
		lua_pushinteger(lua, GL_DELETE_STATUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPILE_STATUS");
		lua_pushinteger(lua, GL_COMPILE_STATUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINK_STATUS");
		lua_pushinteger(lua, GL_LINK_STATUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VALIDATE_STATUS");
		lua_pushinteger(lua, GL_VALIDATE_STATUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INFO_LOG_LENGTH");
		lua_pushinteger(lua, GL_INFO_LOG_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTACHED_SHADERS");
		lua_pushinteger(lua, GL_ATTACHED_SHADERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_UNIFORMS");
		lua_pushinteger(lua, GL_ACTIVE_UNIFORMS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_UNIFORM_MAX_LENGTH");
		lua_pushinteger(lua, GL_ACTIVE_UNIFORM_MAX_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_SOURCE_LENGTH");
		lua_pushinteger(lua, GL_SHADER_SOURCE_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_ATTRIBUTES");
		lua_pushinteger(lua, GL_ACTIVE_ATTRIBUTES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_ATTRIBUTE_MAX_LENGTH");
		lua_pushinteger(lua, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_SHADER_DERIVATIVE_HINT");
		lua_pushinteger(lua, GL_FRAGMENT_SHADER_DERIVATIVE_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADING_LANGUAGE_VERSION");
		lua_pushinteger(lua, GL_SHADING_LANGUAGE_VERSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_PROGRAM");
		lua_pushinteger(lua, GL_CURRENT_PROGRAM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SPRITE_COORD_ORIGIN");
		lua_pushinteger(lua, GL_POINT_SPRITE_COORD_ORIGIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOWER_LEFT");
		lua_pushinteger(lua, GL_LOWER_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UPPER_LEFT");
		lua_pushinteger(lua, GL_UPPER_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_REF");
		lua_pushinteger(lua, GL_STENCIL_BACK_REF);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_VALUE_MASK");
		lua_pushinteger(lua, GL_STENCIL_BACK_VALUE_MASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_WRITEMASK");
		lua_pushinteger(lua, GL_STENCIL_BACK_WRITEMASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_2_1");
		lua_pushinteger(lua, GL_VERSION_2_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_RASTER_SECONDARY_COLOR");
		lua_pushinteger(lua, GL_CURRENT_RASTER_SECONDARY_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_PACK_BUFFER");
		lua_pushinteger(lua, GL_PIXEL_PACK_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_UNPACK_BUFFER");
		lua_pushinteger(lua, GL_PIXEL_UNPACK_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_PACK_BUFFER_BINDING");
		lua_pushinteger(lua, GL_PIXEL_PACK_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_UNPACK_BUFFER_BINDING");
		lua_pushinteger(lua, GL_PIXEL_UNPACK_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT2x3");
		lua_pushinteger(lua, GL_FLOAT_MAT2x3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT2x4");
		lua_pushinteger(lua, GL_FLOAT_MAT2x4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT3x2");
		lua_pushinteger(lua, GL_FLOAT_MAT3x2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT3x4");
		lua_pushinteger(lua, GL_FLOAT_MAT3x4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT4x2");
		lua_pushinteger(lua, GL_FLOAT_MAT4x2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT4x3");
		lua_pushinteger(lua, GL_FLOAT_MAT4x3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB");
		lua_pushinteger(lua, GL_SRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB8");
		lua_pushinteger(lua, GL_SRGB8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB_ALPHA");
		lua_pushinteger(lua, GL_SRGB_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB8_ALPHA8");
		lua_pushinteger(lua, GL_SRGB8_ALPHA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE_ALPHA");
		lua_pushinteger(lua, GL_SLUMINANCE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE8_ALPHA8");
		lua_pushinteger(lua, GL_SLUMINANCE8_ALPHA8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE");
		lua_pushinteger(lua, GL_SLUMINANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE8");
		lua_pushinteger(lua, GL_SLUMINANCE8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SLUMINANCE");
		lua_pushinteger(lua, GL_COMPRESSED_SLUMINANCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SLUMINANCE_ALPHA");
		lua_pushinteger(lua, GL_COMPRESSED_SLUMINANCE_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_3_0");
		lua_pushinteger(lua, GL_VERSION_3_0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE0");
		lua_pushinteger(lua, GL_CLIP_DISTANCE0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE1");
		lua_pushinteger(lua, GL_CLIP_DISTANCE1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE2");
		lua_pushinteger(lua, GL_CLIP_DISTANCE2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE3");
		lua_pushinteger(lua, GL_CLIP_DISTANCE3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE4");
		lua_pushinteger(lua, GL_CLIP_DISTANCE4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE5");
		lua_pushinteger(lua, GL_CLIP_DISTANCE5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPARE_REF_TO_TEXTURE");
		lua_pushinteger(lua, GL_COMPARE_REF_TO_TEXTURE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CLIP_DISTANCES");
		lua_pushinteger(lua, GL_MAX_CLIP_DISTANCES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VARYING_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_VARYING_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT");
		lua_pushinteger(lua, GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAJOR_VERSION");
		lua_pushinteger(lua, GL_MAJOR_VERSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINOR_VERSION");
		lua_pushinteger(lua, GL_MINOR_VERSION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_EXTENSIONS");
		lua_pushinteger(lua, GL_NUM_EXTENSIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTEXT_FLAGS");
		lua_pushinteger(lua, GL_CONTEXT_FLAGS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BUFFER");
		lua_pushinteger(lua, GL_DEPTH_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BUFFER");
		lua_pushinteger(lua, GL_STENCIL_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA32F");
		lua_pushinteger(lua, GL_RGBA32F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB32F");
		lua_pushinteger(lua, GL_RGB32F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16F");
		lua_pushinteger(lua, GL_RGBA16F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16F");
		lua_pushinteger(lua, GL_RGB16F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_INTEGER");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ARRAY_TEXTURE_LAYERS");
		lua_pushinteger(lua, GL_MAX_ARRAY_TEXTURE_LAYERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_PROGRAM_TEXEL_OFFSET");
		lua_pushinteger(lua, GL_MIN_PROGRAM_TEXEL_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEXEL_OFFSET");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEXEL_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_VERTEX_COLOR");
		lua_pushinteger(lua, GL_CLAMP_VERTEX_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_FRAGMENT_COLOR");
		lua_pushinteger(lua, GL_CLAMP_FRAGMENT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_READ_COLOR");
		lua_pushinteger(lua, GL_CLAMP_READ_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIXED_ONLY");
		lua_pushinteger(lua, GL_FIXED_ONLY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RED_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_RED_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GREEN_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_GREEN_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BLUE_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_BLUE_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ALPHA_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_ALPHA_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LUMINANCE_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_LUMINANCE_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTENSITY_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_INTENSITY_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH_TYPE");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_1D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D_ARRAY");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_2D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D_ARRAY");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_1D_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_1D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_2D_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_2D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R11F_G11F_B10F");
		lua_pushinteger(lua, GL_R11F_G11F_B10F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_10F_11F_11F_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_10F_11F_11F_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB9_E5");
		lua_pushinteger(lua, GL_RGB9_E5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_5_9_9_9_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_5_9_9_9_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SHARED_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_SHARED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_MODE");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_VARYINGS");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_VARYINGS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_START");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_START);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_SIZE");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVES_GENERATED");
		lua_pushinteger(lua, GL_PRIMITIVES_GENERATED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RASTERIZER_DISCARD");
		lua_pushinteger(lua, GL_RASTERIZER_DISCARD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLEAVED_ATTRIBS");
		lua_pushinteger(lua, GL_INTERLEAVED_ATTRIBS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARATE_ATTRIBS");
		lua_pushinteger(lua, GL_SEPARATE_ATTRIBS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_BINDING");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA32UI");
		lua_pushinteger(lua, GL_RGBA32UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB32UI");
		lua_pushinteger(lua, GL_RGB32UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16UI");
		lua_pushinteger(lua, GL_RGBA16UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16UI");
		lua_pushinteger(lua, GL_RGB16UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8UI");
		lua_pushinteger(lua, GL_RGBA8UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8UI");
		lua_pushinteger(lua, GL_RGB8UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA32I");
		lua_pushinteger(lua, GL_RGBA32I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB32I");
		lua_pushinteger(lua, GL_RGB32I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16I");
		lua_pushinteger(lua, GL_RGBA16I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16I");
		lua_pushinteger(lua, GL_RGB16I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8I");
		lua_pushinteger(lua, GL_RGBA8I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8I");
		lua_pushinteger(lua, GL_RGB8I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_INTEGER");
		lua_pushinteger(lua, GL_RED_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_INTEGER");
		lua_pushinteger(lua, GL_GREEN_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_INTEGER");
		lua_pushinteger(lua, GL_BLUE_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_INTEGER");
		lua_pushinteger(lua, GL_ALPHA_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_INTEGER");
		lua_pushinteger(lua, GL_RGB_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_INTEGER");
		lua_pushinteger(lua, GL_RGBA_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGR_INTEGER");
		lua_pushinteger(lua, GL_BGR_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA_INTEGER");
		lua_pushinteger(lua, GL_BGRA_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_ARRAY");
		lua_pushinteger(lua, GL_SAMPLER_1D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_ARRAY");
		lua_pushinteger(lua, GL_SAMPLER_2D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_ARRAY_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_1D_ARRAY_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_ARRAY_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_2D_ARRAY_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_VEC2");
		lua_pushinteger(lua, GL_UNSIGNED_INT_VEC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_VEC3");
		lua_pushinteger(lua, GL_UNSIGNED_INT_VEC3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_VEC4");
		lua_pushinteger(lua, GL_UNSIGNED_INT_VEC4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_1D");
		lua_pushinteger(lua, GL_INT_SAMPLER_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_3D");
		lua_pushinteger(lua, GL_INT_SAMPLER_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_CUBE");
		lua_pushinteger(lua, GL_INT_SAMPLER_CUBE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_1D_ARRAY");
		lua_pushinteger(lua, GL_INT_SAMPLER_1D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_ARRAY");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_1D");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_3D");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_CUBE");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_CUBE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_1D_ARRAY");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_ARRAY");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_WAIT");
		lua_pushinteger(lua, GL_QUERY_WAIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_NO_WAIT");
		lua_pushinteger(lua, GL_QUERY_NO_WAIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_BY_REGION_WAIT");
		lua_pushinteger(lua, GL_QUERY_BY_REGION_WAIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_BY_REGION_NO_WAIT");
		lua_pushinteger(lua, GL_QUERY_BY_REGION_NO_WAIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_3_1");
		lua_pushinteger(lua, GL_VERSION_3_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RECTANGLE");
		lua_pushinteger(lua, GL_TEXTURE_RECTANGLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_RECTANGLE");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_RECTANGLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_RECTANGLE");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_RECTANGLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_RECTANGLE_TEXTURE_SIZE");
		lua_pushinteger(lua, GL_MAX_RECTANGLE_TEXTURE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_RECT");
		lua_pushinteger(lua, GL_SAMPLER_2D_RECT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_RECT_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_2D_RECT_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_BUFFER_SIZE");
		lua_pushinteger(lua, GL_MAX_TEXTURE_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_BUFFER");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_DATA_STORE_BINDING");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_DATA_STORE_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_FORMAT");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_BUFFER");
		lua_pushinteger(lua, GL_SAMPLER_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_RECT");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_RECT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_BUFFER");
		lua_pushinteger(lua, GL_INT_SAMPLER_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_RECT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_RECT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_BUFFER");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_SNORM");
		lua_pushinteger(lua, GL_RED_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG_SNORM");
		lua_pushinteger(lua, GL_RG_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_SNORM");
		lua_pushinteger(lua, GL_RGB_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_SNORM");
		lua_pushinteger(lua, GL_RGBA_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R8_SNORM");
		lua_pushinteger(lua, GL_R8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG8_SNORM");
		lua_pushinteger(lua, GL_RG8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8_SNORM");
		lua_pushinteger(lua, GL_RGB8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8_SNORM");
		lua_pushinteger(lua, GL_RGBA8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R16_SNORM");
		lua_pushinteger(lua, GL_R16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16_SNORM");
		lua_pushinteger(lua, GL_RG16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16_SNORM");
		lua_pushinteger(lua, GL_RGB16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16_SNORM");
		lua_pushinteger(lua, GL_RGBA16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_NORMALIZED");
		lua_pushinteger(lua, GL_SIGNED_NORMALIZED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVE_RESTART");
		lua_pushinteger(lua, GL_PRIMITIVE_RESTART);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVE_RESTART_INDEX");
		lua_pushinteger(lua, GL_PRIMITIVE_RESTART_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_ACCESS_FLAGS");
		lua_pushinteger(lua, GL_BUFFER_ACCESS_FLAGS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_MAP_LENGTH");
		lua_pushinteger(lua, GL_BUFFER_MAP_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_MAP_OFFSET");
		lua_pushinteger(lua, GL_BUFFER_MAP_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_3_2");
		lua_pushinteger(lua, GL_VERSION_3_2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTEXT_CORE_PROFILE_BIT");
		lua_pushinteger(lua, GL_CONTEXT_CORE_PROFILE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTEXT_COMPATIBILITY_PROFILE_BIT");
		lua_pushinteger(lua, GL_CONTEXT_COMPATIBILITY_PROFILE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINES_ADJACENCY");
		lua_pushinteger(lua, GL_LINES_ADJACENCY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STRIP_ADJACENCY");
		lua_pushinteger(lua, GL_LINE_STRIP_ADJACENCY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLES_ADJACENCY");
		lua_pushinteger(lua, GL_TRIANGLES_ADJACENCY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_STRIP_ADJACENCY");
		lua_pushinteger(lua, GL_TRIANGLE_STRIP_ADJACENCY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_POINT_SIZE");
		lua_pushinteger(lua, GL_PROGRAM_POINT_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_VERTICES_OUT");
		lua_pushinteger(lua, GL_GEOMETRY_VERTICES_OUT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_INPUT_TYPE");
		lua_pushinteger(lua, GL_GEOMETRY_INPUT_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_OUTPUT_TYPE");
		lua_pushinteger(lua, GL_GEOMETRY_OUTPUT_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_TEXTURE_IMAGE_UNITS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_LAYERED");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_LAYERED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_SHADER");
		lua_pushinteger(lua, GL_GEOMETRY_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_OUTPUT_VERTICES");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_OUTPUT_VERTICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_OUTPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_VERTEX_OUTPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_INPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_INPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_OUTPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_OUTPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_INPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_INPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTEXT_PROFILE_MASK");
		lua_pushinteger(lua, GL_CONTEXT_PROFILE_MASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_3_3");
		lua_pushinteger(lua, GL_VERSION_3_3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_DIVISOR");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_DIVISOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10_A2UI");
		lua_pushinteger(lua, GL_RGB10_A2UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_4_0");
		lua_pushinteger(lua, GL_VERSION_4_0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_SHADING");
		lua_pushinteger(lua, GL_SAMPLE_SHADING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_SAMPLE_SHADING_VALUE");
		lua_pushinteger(lua, GL_MIN_SAMPLE_SHADING_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_PROGRAM_TEXTURE_GATHER_OFFSET");
		lua_pushinteger(lua, GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEXTURE_GATHER_OFFSET");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS");
		// lua_pushinteger(lua, GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_CUBE_MAP_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_CUBE_MAP_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP_ARRAY");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_MAP_ARRAY");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_MAP_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_MAP_ARRAY_SHADOW");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_CUBE_MAP_ARRAY");
		lua_pushinteger(lua, GL_INT_SAMPLER_CUBE_MAP_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERSION_4_1");
		lua_pushinteger(lua, GL_VERSION_4_1);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "VERSION_4_2");
		// lua_pushinteger(lua, GL_VERSION_4_2);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_RGBA_BPTC_UNORM");
		// lua_pushinteger(lua, GL_COMPRESSED_RGBA_BPTC_UNORM);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA_BPTC_UNORM");
		// lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_RGB_BPTC_SIGNED_FLOAT");
		// lua_pushinteger(lua, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT");
		// lua_pushinteger(lua, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COPY_READ_BUFFER_BINDING");
		// lua_pushinteger(lua, GL_COPY_READ_BUFFER_BINDING);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COPY_WRITE_BUFFER_BINDING");
		// lua_pushinteger(lua, GL_COPY_WRITE_BUFFER_BINDING);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "VERSION_4_3");
		// lua_pushinteger(lua, GL_VERSION_4_3);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "NUM_SHADING_LANGUAGE_VERSIONS");
		// lua_pushinteger(lua, GL_NUM_SHADING_LANGUAGE_VERSIONS);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_LONG");
		// lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_LONG);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "VERSION_4_4");
		// lua_pushinteger(lua, GL_VERSION_4_4);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED");
		// lua_pushinteger(lua, GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "MAX_VERTEX_ATTRIB_STRIDE");
		// lua_pushinteger(lua, GL_MAX_VERTEX_ATTRIB_STRIDE);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "TEXTURE_BUFFER_BINDING");
		// lua_pushinteger(lua, GL_TEXTURE_BUFFER_BINDING);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "VERSION_4_5");
		// lua_pushinteger(lua, GL_VERSION_4_5);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "3DFX_multisample");
		lua_pushinteger(lua, GL_3DFX_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_3DFX");
		lua_pushinteger(lua, GL_MULTISAMPLE_3DFX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_BUFFERS_3DFX");
		lua_pushinteger(lua, GL_SAMPLE_BUFFERS_3DFX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES_3DFX");
		lua_pushinteger(lua, GL_SAMPLES_3DFX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_BIT_3DFX");
		lua_pushinteger(lua, GL_MULTISAMPLE_BIT_3DFX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3DFX_tbuffer");
		lua_pushinteger(lua, GL_3DFX_tbuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "3DFX_texture_compression_FXT1");
		lua_pushinteger(lua, GL_3DFX_texture_compression_FXT1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGB_FXT1_3DFX");
		lua_pushinteger(lua, GL_COMPRESSED_RGB_FXT1_3DFX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA_FXT1_3DFX");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA_FXT1_3DFX);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_blend_minmax_factor");
		// lua_pushinteger(lua, GL_AMD_blend_minmax_factor);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "FACTOR_MIN_AMD");
		// lua_pushinteger(lua, GL_FACTOR_MIN_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "FACTOR_MAX_AMD");
		// lua_pushinteger(lua, GL_FACTOR_MAX_AMD);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_conservative_depth");
		lua_pushinteger(lua, GL_AMD_conservative_depth);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_debug_output");
		lua_pushinteger(lua, GL_AMD_debug_output);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "MAX_DEBUG_MESSAGE_LENGTH_AMD");
		// lua_pushinteger(lua, GL_MAX_DEBUG_MESSAGE_LENGTH_AMD);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DEBUG_LOGGED_MESSAGES_AMD");
		lua_pushinteger(lua, GL_MAX_DEBUG_LOGGED_MESSAGES_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_LOGGED_MESSAGES_AMD");
		lua_pushinteger(lua, GL_DEBUG_LOGGED_MESSAGES_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SEVERITY_HIGH_AMD");
		lua_pushinteger(lua, GL_DEBUG_SEVERITY_HIGH_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SEVERITY_MEDIUM_AMD");
		lua_pushinteger(lua, GL_DEBUG_SEVERITY_MEDIUM_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SEVERITY_LOW_AMD");
		lua_pushinteger(lua, GL_DEBUG_SEVERITY_LOW_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_API_ERROR_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_API_ERROR_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_WINDOW_SYSTEM_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_DEPRECATION_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_DEPRECATION_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_PERFORMANCE_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_PERFORMANCE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_SHADER_COMPILER_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_APPLICATION_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_APPLICATION_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CATEGORY_OTHER_AMD");
		lua_pushinteger(lua, GL_DEBUG_CATEGORY_OTHER_AMD);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_depth_clamp_separate");
		// lua_pushinteger(lua, GL_AMD_depth_clamp_separate);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "DEPTH_CLAMP_NEAR_AMD");
		// lua_pushinteger(lua, GL_DEPTH_CLAMP_NEAR_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "DEPTH_CLAMP_FAR_AMD");
		// lua_pushinteger(lua, GL_DEPTH_CLAMP_FAR_AMD);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_draw_buffers_blend");
		lua_pushinteger(lua, GL_AMD_draw_buffers_blend);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_gcn_shader");
		// lua_pushinteger(lua, GL_AMD_gcn_shader);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_gpu_shader_int64");
		// lua_pushinteger(lua, GL_AMD_gpu_shader_int64);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_interleaved_elements");
		// lua_pushinteger(lua, GL_AMD_interleaved_elements);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "RED");
		lua_pushinteger(lua, GL_RED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN");
		lua_pushinteger(lua, GL_GREEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE");
		lua_pushinteger(lua, GL_BLUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA");
		lua_pushinteger(lua, GL_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG8UI");
		lua_pushinteger(lua, GL_RG8UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16UI");
		lua_pushinteger(lua, GL_RG16UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8UI");
		lua_pushinteger(lua, GL_RGBA8UI);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "VERTEX_ELEMENT_SWIZZLE_AMD");
		// lua_pushinteger(lua, GL_VERTEX_ELEMENT_SWIZZLE_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "VERTEX_ID_SWIZZLE_AMD");
		// lua_pushinteger(lua, GL_VERTEX_ID_SWIZZLE_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_multi_draw_indirect");
		// lua_pushinteger(lua, GL_AMD_multi_draw_indirect);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_name_gen_delete");
		lua_pushinteger(lua, GL_AMD_name_gen_delete);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DATA_BUFFER_AMD");
		lua_pushinteger(lua, GL_DATA_BUFFER_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERFORMANCE_MONITOR_AMD");
		lua_pushinteger(lua, GL_PERFORMANCE_MONITOR_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_OBJECT_AMD");
		lua_pushinteger(lua, GL_QUERY_OBJECT_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_OBJECT_AMD");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_OBJECT_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_OBJECT_AMD");
		lua_pushinteger(lua, GL_SAMPLER_OBJECT_AMD);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_occlusion_query_event");
		// lua_pushinteger(lua, GL_AMD_occlusion_query_event);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_DEPTH_PASS_EVENT_BIT_AMD");
		// lua_pushinteger(lua, GL_QUERY_DEPTH_PASS_EVENT_BIT_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_DEPTH_FAIL_EVENT_BIT_AMD");
		// lua_pushinteger(lua, GL_QUERY_DEPTH_FAIL_EVENT_BIT_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_STENCIL_FAIL_EVENT_BIT_AMD");
		// lua_pushinteger(lua, GL_QUERY_STENCIL_FAIL_EVENT_BIT_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT_AMD");
		// lua_pushinteger(lua, GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "OCCLUSION_QUERY_EVENT_MASK_AMD");
		// lua_pushinteger(lua, GL_OCCLUSION_QUERY_EVENT_MASK_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_ALL_EVENT_BITS_AMD");
		// lua_pushinteger(lua, GL_QUERY_ALL_EVENT_BITS_AMD);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_performance_monitor");
		lua_pushinteger(lua, GL_AMD_performance_monitor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COUNTER_TYPE_AMD");
		lua_pushinteger(lua, GL_COUNTER_TYPE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COUNTER_RANGE_AMD");
		lua_pushinteger(lua, GL_COUNTER_RANGE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT64_AMD");
		lua_pushinteger(lua, GL_UNSIGNED_INT64_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERCENTAGE_AMD");
		lua_pushinteger(lua, GL_PERCENTAGE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERFMON_RESULT_AVAILABLE_AMD");
		lua_pushinteger(lua, GL_PERFMON_RESULT_AVAILABLE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERFMON_RESULT_SIZE_AMD");
		lua_pushinteger(lua, GL_PERFMON_RESULT_SIZE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERFMON_RESULT_AMD");
		lua_pushinteger(lua, GL_PERFMON_RESULT_AMD);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_pinned_memory");
//		lua_pushinteger(lua, GL_AMD_pinned_memory);
//		lua_settable(lua, -3);
		// lua_pushstring(lua, "EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD");
		// lua_pushinteger(lua, GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_query_buffer_object");
		// lua_pushinteger(lua, GL_AMD_query_buffer_object);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_BUFFER_AMD");
		// lua_pushinteger(lua, GL_QUERY_BUFFER_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_BUFFER_BINDING_AMD");
		// lua_pushinteger(lua, GL_QUERY_BUFFER_BINDING_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "QUERY_RESULT_NO_WAIT_AMD");
		// lua_pushinteger(lua, GL_QUERY_RESULT_NO_WAIT_AMD);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "AMD_sample_positions");
		// lua_pushinteger(lua, GL_AMD_sample_positions);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "SUBSAMPLE_DISTANCE_AMD");
		// lua_pushinteger(lua, GL_SUBSAMPLE_DISTANCE_AMD);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_seamless_cubemap_per_texture");
		lua_pushinteger(lua, GL_AMD_seamless_cubemap_per_texture);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "TEXTURE_CUBE_MAP_SEAMLESS_ARB");
		// lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB);
		// lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_shader_atomic_counter_ops");
//		lua_pushinteger(lua, GL_AMD_shader_atomic_counter_ops);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_shader_stencil_export");
		lua_pushinteger(lua, GL_AMD_shader_stencil_export);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_shader_stencil_value_export");
//		lua_pushinteger(lua, GL_AMD_shader_stencil_value_export);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_shader_trinary_minmax");
//		lua_pushinteger(lua, GL_AMD_shader_trinary_minmax);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_sparse_texture");
//		lua_pushinteger(lua, GL_AMD_sparse_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_STORAGE_SPARSE_BIT_AMD");
//		lua_pushinteger(lua, GL_TEXTURE_STORAGE_SPARSE_BIT_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_X_AMD");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_X_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_Y_AMD");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_Y_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_Z_AMD");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_Z_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SPARSE_TEXTURE_SIZE_AMD");
//		lua_pushinteger(lua, GL_MAX_SPARSE_TEXTURE_SIZE_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SPARSE_3D_TEXTURE_SIZE_AMD");
//		lua_pushinteger(lua, GL_MAX_SPARSE_3D_TEXTURE_SIZE_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SPARSE_ARRAY_TEXTURE_LAYERS");
//		lua_pushinteger(lua, GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_SPARSE_LEVEL_AMD");
//		lua_pushinteger(lua, GL_MIN_SPARSE_LEVEL_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_LOD_WARNING_AMD");
//		lua_pushinteger(lua, GL_MIN_LOD_WARNING_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_stencil_operation_extended");
//		lua_pushinteger(lua, GL_AMD_stencil_operation_extended);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SET_AMD");
//		lua_pushinteger(lua, GL_SET_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REPLACE_VALUE_AMD");
//		lua_pushinteger(lua, GL_REPLACE_VALUE_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STENCIL_OP_VALUE_AMD");
//		lua_pushinteger(lua, GL_STENCIL_OP_VALUE_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STENCIL_BACK_OP_VALUE_AMD");
//		lua_pushinteger(lua, GL_STENCIL_BACK_OP_VALUE_AMD);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_texture_texture4");
		lua_pushinteger(lua, GL_AMD_texture_texture4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AMD_transform_feedback3_lines_triangles");
		lua_pushinteger(lua, GL_AMD_transform_feedback3_lines_triangles);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_transform_feedback4");
//		lua_pushinteger(lua, GL_AMD_transform_feedback4);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STREAM_RASTERIZATION_AMD");
//		lua_pushinteger(lua, GL_STREAM_RASTERIZATION_AMD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_vertex_shader_layer");
//		lua_pushinteger(lua, GL_AMD_vertex_shader_layer);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_vertex_shader_tessellator");
//		lua_pushinteger(lua, GL_AMD_vertex_shader_tessellator);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_BUFFER_AMD");
		lua_pushinteger(lua, GL_SAMPLER_BUFFER_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_BUFFER_AMD");
		lua_pushinteger(lua, GL_INT_SAMPLER_BUFFER_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_BUFFER_AMD");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESSELLATION_MODE_AMD");
		lua_pushinteger(lua, GL_TESSELLATION_MODE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESSELLATION_FACTOR_AMD");
		lua_pushinteger(lua, GL_TESSELLATION_FACTOR_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DISCRETE_AMD");
		lua_pushinteger(lua, GL_DISCRETE_AMD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTINUOUS_AMD");
		lua_pushinteger(lua, GL_CONTINUOUS_AMD);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "AMD_vertex_shader_viewport_index");
//		lua_pushinteger(lua, GL_AMD_vertex_shader_viewport_index);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_depth_texture");
//		lua_pushinteger(lua, GL_ANGLE_depth_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_framebuffer_blit");
//		lua_pushinteger(lua, GL_ANGLE_framebuffer_blit);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DRAW_FRAMEBUFFER_BINDING_ANGLE");
//		lua_pushinteger(lua, GL_DRAW_FRAMEBUFFER_BINDING_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "READ_FRAMEBUFFER_ANGLE");
//		lua_pushinteger(lua, GL_READ_FRAMEBUFFER_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DRAW_FRAMEBUFFER_ANGLE");
//		lua_pushinteger(lua, GL_DRAW_FRAMEBUFFER_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "READ_FRAMEBUFFER_BINDING_ANGLE");
//		lua_pushinteger(lua, GL_READ_FRAMEBUFFER_BINDING_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_framebuffer_multisample");
//		lua_pushinteger(lua, GL_ANGLE_framebuffer_multisample);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RENDERBUFFER_SAMPLES_ANGLE");
//		lua_pushinteger(lua, GL_RENDERBUFFER_SAMPLES_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SAMPLES_ANGLE");
//		lua_pushinteger(lua, GL_MAX_SAMPLES_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_instanced_arrays");
//		lua_pushinteger(lua, GL_ANGLE_instanced_arrays);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE");
//		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_pack_reverse_row_order");
//		lua_pushinteger(lua, GL_ANGLE_pack_reverse_row_order);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PACK_REVERSE_ROW_ORDER_ANGLE");
//		lua_pushinteger(lua, GL_PACK_REVERSE_ROW_ORDER_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_program_binary");
//		lua_pushinteger(lua, GL_ANGLE_program_binary);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM_BINARY_ANGLE");
//		lua_pushinteger(lua, GL_PROGRAM_BINARY_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_texture_compression_dxt1");
//		lua_pushinteger(lua, GL_ANGLE_texture_compression_dxt1);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGB_S3TC_DXT1_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGB_S3TC_DXT1_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT1_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT1_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT3_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT5_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_texture_compression_dxt3");
//		lua_pushinteger(lua, GL_ANGLE_texture_compression_dxt3);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGB_S3TC_DXT1_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGB_S3TC_DXT1_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT1_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT1_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT3_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT5_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_texture_compression_dxt5");
//		lua_pushinteger(lua, GL_ANGLE_texture_compression_dxt5);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGB_S3TC_DXT1_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGB_S3TC_DXT1_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT1_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT1_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT3_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT5_ANGLE");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_texture_usage");
//		lua_pushinteger(lua, GL_ANGLE_texture_usage);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_USAGE_ANGLE");
//		lua_pushinteger(lua, GL_TEXTURE_USAGE_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_ANGLE");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_timer_query");
//		lua_pushinteger(lua, GL_ANGLE_timer_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_COUNTER_BITS_ANGLE");
//		lua_pushinteger(lua, GL_QUERY_COUNTER_BITS_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CURRENT_QUERY_ANGLE");
//		lua_pushinteger(lua, GL_CURRENT_QUERY_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_RESULT_ANGLE");
//		lua_pushinteger(lua, GL_QUERY_RESULT_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_RESULT_AVAILABLE_ANGLE");
//		lua_pushinteger(lua, GL_QUERY_RESULT_AVAILABLE_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TIME_ELAPSED_ANGLE");
//		lua_pushinteger(lua, GL_TIME_ELAPSED_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TIMESTAMP_ANGLE");
//		lua_pushinteger(lua, GL_TIMESTAMP_ANGLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANGLE_translated_shader_source");
//		lua_pushinteger(lua, GL_ANGLE_translated_shader_source);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE");
//		lua_pushinteger(lua, GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_aux_depth_stencil");
		lua_pushinteger(lua, GL_APPLE_aux_depth_stencil);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AUX_DEPTH_STENCIL_APPLE");
		lua_pushinteger(lua, GL_AUX_DEPTH_STENCIL_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_client_storage");
		lua_pushinteger(lua, GL_APPLE_client_storage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_CLIENT_STORAGE_APPLE");
		lua_pushinteger(lua, GL_UNPACK_CLIENT_STORAGE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_element_array");
		lua_pushinteger(lua, GL_APPLE_element_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_APPLE");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_TYPE_APPLE");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_TYPE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_POINTER_APPLE");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_POINTER_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_fence");
		lua_pushinteger(lua, GL_APPLE_fence);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_PIXELS_APPLE");
		lua_pushinteger(lua, GL_DRAW_PIXELS_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FENCE_APPLE");
		lua_pushinteger(lua, GL_FENCE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_float_pixels");
		lua_pushinteger(lua, GL_APPLE_float_pixels);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_APPLE");
		lua_pushinteger(lua, GL_HALF_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_FLOAT32_APPLE");
		lua_pushinteger(lua, GL_RGBA_FLOAT32_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_FLOAT32_APPLE");
		lua_pushinteger(lua, GL_RGB_FLOAT32_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_FLOAT32_APPLE");
		lua_pushinteger(lua, GL_ALPHA_FLOAT32_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY_FLOAT32_APPLE");
		lua_pushinteger(lua, GL_INTENSITY_FLOAT32_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_FLOAT32_APPLE");
		lua_pushinteger(lua, GL_LUMINANCE_FLOAT32_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA_FLOAT32_APPLE");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_FLOAT32_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_FLOAT16_APPLE");
		lua_pushinteger(lua, GL_RGBA_FLOAT16_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_FLOAT16_APPLE");
		lua_pushinteger(lua, GL_RGB_FLOAT16_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_FLOAT16_APPLE");
		lua_pushinteger(lua, GL_ALPHA_FLOAT16_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY_FLOAT16_APPLE");
		lua_pushinteger(lua, GL_INTENSITY_FLOAT16_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_FLOAT16_APPLE");
		lua_pushinteger(lua, GL_LUMINANCE_FLOAT16_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA_FLOAT16_APPLE");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_FLOAT16_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_FLOAT_APPLE");
		lua_pushinteger(lua, GL_COLOR_FLOAT_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_flush_buffer_range");
		lua_pushinteger(lua, GL_APPLE_flush_buffer_range);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_SERIALIZED_MODIFY_APPLE");
		lua_pushinteger(lua, GL_BUFFER_SERIALIZED_MODIFY_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_FLUSHING_UNMAP_APPLE");
		lua_pushinteger(lua, GL_BUFFER_FLUSHING_UNMAP_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_object_purgeable");
		lua_pushinteger(lua, GL_APPLE_object_purgeable);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_OBJECT_APPLE");
		lua_pushinteger(lua, GL_BUFFER_OBJECT_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RELEASED_APPLE");
		lua_pushinteger(lua, GL_RELEASED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VOLATILE_APPLE");
		lua_pushinteger(lua, GL_VOLATILE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RETAINED_APPLE");
		lua_pushinteger(lua, GL_RETAINED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNDEFINED_APPLE");
		lua_pushinteger(lua, GL_UNDEFINED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PURGEABLE_APPLE");
		lua_pushinteger(lua, GL_PURGEABLE_APPLE);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "APPLE_pixel_buffer");
//		lua_pushinteger(lua, GL_APPLE_pixel_buffer);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_PBUFFER_VIEWPORT_DIMS_APPLE");
//		lua_pushinteger(lua, GL_MIN_PBUFFER_VIEWPORT_DIMS_APPLE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_rgb_422");
		lua_pushinteger(lua, GL_APPLE_rgb_422);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_8_8_APPLE");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_8_8_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_8_8_REV_APPLE");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_8_8_REV_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_422_APPLE");
		lua_pushinteger(lua, GL_RGB_422_APPLE);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGB_RAW_422_APPLE");
//		lua_pushinteger(lua, GL_RGB_RAW_422_APPLE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_row_bytes");
		lua_pushinteger(lua, GL_APPLE_row_bytes);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_ROW_BYTES_APPLE");
		lua_pushinteger(lua, GL_PACK_ROW_BYTES_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_ROW_BYTES_APPLE");
		lua_pushinteger(lua, GL_UNPACK_ROW_BYTES_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_specular_vector");
		lua_pushinteger(lua, GL_APPLE_specular_vector);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_SPECULAR_VECTOR_APPLE");
		lua_pushinteger(lua, GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_texture_range");
		lua_pushinteger(lua, GL_APPLE_texture_range);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RANGE_LENGTH_APPLE");
		lua_pushinteger(lua, GL_TEXTURE_RANGE_LENGTH_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RANGE_POINTER_APPLE");
		lua_pushinteger(lua, GL_TEXTURE_RANGE_POINTER_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_STORAGE_HINT_APPLE");
		lua_pushinteger(lua, GL_TEXTURE_STORAGE_HINT_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STORAGE_PRIVATE_APPLE");
		lua_pushinteger(lua, GL_STORAGE_PRIVATE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STORAGE_CACHED_APPLE");
		lua_pushinteger(lua, GL_STORAGE_CACHED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STORAGE_SHARED_APPLE");
		lua_pushinteger(lua, GL_STORAGE_SHARED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_transform_hint");
		lua_pushinteger(lua, GL_APPLE_transform_hint);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_HINT_APPLE");
		lua_pushinteger(lua, GL_TRANSFORM_HINT_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_vertex_array_object");
		lua_pushinteger(lua, GL_APPLE_vertex_array_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_BINDING_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_BINDING_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_vertex_array_range");
		lua_pushinteger(lua, GL_APPLE_vertex_array_range);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_LENGTH_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_STORAGE_HINT_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_STORAGE_HINT_APPLE);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE");
//		lua_pushinteger(lua, GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_POINTER_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_POINTER_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STORAGE_CLIENT_APPLE");
		lua_pushinteger(lua, GL_STORAGE_CLIENT_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STORAGE_CACHED_APPLE");
		lua_pushinteger(lua, GL_STORAGE_CACHED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STORAGE_SHARED_APPLE");
		lua_pushinteger(lua, GL_STORAGE_SHARED_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_vertex_program_evaluators");
		lua_pushinteger(lua, GL_APPLE_vertex_program_evaluators);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP1_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP1_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP2_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP2_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP1_SIZE_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP1_COEFF_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP1_ORDER_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP1_DOMAIN_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP2_SIZE_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP2_COEFF_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP2_ORDER_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_MAP2_DOMAIN_APPLE");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "APPLE_ycbcr_422");
		lua_pushinteger(lua, GL_APPLE_ycbcr_422);
		lua_settable(lua, -3);
		lua_pushstring(lua, "YCBCR_422_APPLE");
		lua_pushinteger(lua, GL_YCBCR_422_APPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_ES2_compatibility");
		lua_pushinteger(lua, GL_ARB_ES2_compatibility);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIXED");
		lua_pushinteger(lua, GL_FIXED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMPLEMENTATION_COLOR_READ_TYPE");
		lua_pushinteger(lua, GL_IMPLEMENTATION_COLOR_READ_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMPLEMENTATION_COLOR_READ_FORMAT");
		lua_pushinteger(lua, GL_IMPLEMENTATION_COLOR_READ_FORMAT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGB565");
//		lua_pushinteger(lua, GL_RGB565);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "LOW_FLOAT");
		lua_pushinteger(lua, GL_LOW_FLOAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MEDIUM_FLOAT");
		lua_pushinteger(lua, GL_MEDIUM_FLOAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HIGH_FLOAT");
		lua_pushinteger(lua, GL_HIGH_FLOAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOW_INT");
		lua_pushinteger(lua, GL_LOW_INT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MEDIUM_INT");
		lua_pushinteger(lua, GL_MEDIUM_INT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HIGH_INT");
		lua_pushinteger(lua, GL_HIGH_INT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_BINARY_FORMATS");
//		lua_pushinteger(lua, GL_SHADER_BINARY_FORMATS);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_SHADER_BINARY_FORMATS");
		lua_pushinteger(lua, GL_NUM_SHADER_BINARY_FORMATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_COMPILER");
		lua_pushinteger(lua, GL_SHADER_COMPILER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_UNIFORM_VECTORS");
		lua_pushinteger(lua, GL_MAX_VERTEX_UNIFORM_VECTORS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VARYING_VECTORS");
		lua_pushinteger(lua, GL_MAX_VARYING_VECTORS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_UNIFORM_VECTORS");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_UNIFORM_VECTORS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_ES3_1_compatibility");
//		lua_pushinteger(lua, GL_ARB_ES3_1_compatibility);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_ES3_compatibility");
//		lua_pushinteger(lua, GL_ARB_ES3_compatibility);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_IMMUTABLE_LEVELS");
//		lua_pushinteger(lua, GL_TEXTURE_IMMUTABLE_LEVELS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PRIMITIVE_RESTART_FIXED_INDEX");
//		lua_pushinteger(lua, GL_PRIMITIVE_RESTART_FIXED_INDEX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ANY_SAMPLES_PASSED_CONSERVATIVE");
//		lua_pushinteger(lua, GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_ELEMENT_INDEX");
//		lua_pushinteger(lua, GL_MAX_ELEMENT_INDEX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_R11_EAC");
//		lua_pushinteger(lua, GL_COMPRESSED_R11_EAC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SIGNED_R11_EAC");
//		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_R11_EAC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RG11_EAC");
//		lua_pushinteger(lua, GL_COMPRESSED_RG11_EAC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SIGNED_RG11_EAC");
//		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_RG11_EAC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGB8_ETC2");
//		lua_pushinteger(lua, GL_COMPRESSED_RGB8_ETC2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ETC2");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ETC2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2");
//		lua_pushinteger(lua, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA8_ETC2_EAC");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA8_ETC2_EAC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ETC2_EAC");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_arrays_of_arrays");
//		lua_pushinteger(lua, GL_ARB_arrays_of_arrays);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_base_instance");
//		lua_pushinteger(lua, GL_ARB_base_instance);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_bindless_texture");
//		lua_pushinteger(lua, GL_ARB_bindless_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT64_ARB");
//		lua_pushinteger(lua, GL_UNSIGNED_INT64_ARB);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_blend_func_extended");
		lua_pushinteger(lua, GL_ARB_blend_func_extended);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRC1_COLOR");
		lua_pushinteger(lua, GL_SRC1_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_SRC1_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_SRC1_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_SRC1_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_SRC1_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DUAL_SOURCE_DRAW_BUFFERS");
		lua_pushinteger(lua, GL_MAX_DUAL_SOURCE_DRAW_BUFFERS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_buffer_storage");
//		lua_pushinteger(lua, GL_ARB_buffer_storage);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_READ_BIT");
		lua_pushinteger(lua, GL_MAP_READ_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_WRITE_BIT");
		lua_pushinteger(lua, GL_MAP_WRITE_BIT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAP_PERSISTENT_BIT");
//		lua_pushinteger(lua, GL_MAP_PERSISTENT_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAP_COHERENT_BIT");
//		lua_pushinteger(lua, GL_MAP_COHERENT_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DYNAMIC_STORAGE_BIT");
//		lua_pushinteger(lua, GL_DYNAMIC_STORAGE_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLIENT_STORAGE_BIT");
//		lua_pushinteger(lua, GL_CLIENT_STORAGE_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLIENT_MAPPED_BUFFER_BARRIER_BIT");
//		lua_pushinteger(lua, GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_IMMUTABLE_STORAGE");
//		lua_pushinteger(lua, GL_BUFFER_IMMUTABLE_STORAGE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_STORAGE_FLAGS");
//		lua_pushinteger(lua, GL_BUFFER_STORAGE_FLAGS);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_cl_event");
		lua_pushinteger(lua, GL_ARB_cl_event);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_CL_EVENT_ARB");
		lua_pushinteger(lua, GL_SYNC_CL_EVENT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_CL_EVENT_COMPLETE_ARB");
		lua_pushinteger(lua, GL_SYNC_CL_EVENT_COMPLETE_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_clear_buffer_object");
//		lua_pushinteger(lua, GL_ARB_clear_buffer_object);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_clear_texture");
//		lua_pushinteger(lua, GL_ARB_clear_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLEAR_TEXTURE");
//		lua_pushinteger(lua, GL_CLEAR_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_clip_control");
//		lua_pushinteger(lua, GL_ARB_clip_control);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "LOWER_LEFT");
		lua_pushinteger(lua, GL_LOWER_LEFT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UPPER_LEFT");
		lua_pushinteger(lua, GL_UPPER_LEFT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLIP_ORIGIN");
//		lua_pushinteger(lua, GL_CLIP_ORIGIN);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLIP_DEPTH_MODE");
//		lua_pushinteger(lua, GL_CLIP_DEPTH_MODE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NEGATIVE_ONE_TO_ONE");
//		lua_pushinteger(lua, GL_NEGATIVE_ONE_TO_ONE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ZERO_TO_ONE");
//		lua_pushinteger(lua, GL_ZERO_TO_ONE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_color_buffer_float");
		lua_pushinteger(lua, GL_ARB_color_buffer_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_FLOAT_MODE_ARB");
		lua_pushinteger(lua, GL_RGBA_FLOAT_MODE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_VERTEX_COLOR_ARB");
		lua_pushinteger(lua, GL_CLAMP_VERTEX_COLOR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_FRAGMENT_COLOR_ARB");
		lua_pushinteger(lua, GL_CLAMP_FRAGMENT_COLOR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_READ_COLOR_ARB");
		lua_pushinteger(lua, GL_CLAMP_READ_COLOR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIXED_ONLY_ARB");
		lua_pushinteger(lua, GL_FIXED_ONLY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_compatibility");
		lua_pushinteger(lua, GL_ARB_compatibility);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_compressed_texture_pixel_storage");
//		lua_pushinteger(lua, GL_ARB_compressed_texture_pixel_storage);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNPACK_COMPRESSED_BLOCK_WIDTH");
//		lua_pushinteger(lua, GL_UNPACK_COMPRESSED_BLOCK_WIDTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNPACK_COMPRESSED_BLOCK_HEIGHT");
//		lua_pushinteger(lua, GL_UNPACK_COMPRESSED_BLOCK_HEIGHT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNPACK_COMPRESSED_BLOCK_DEPTH");
//		lua_pushinteger(lua, GL_UNPACK_COMPRESSED_BLOCK_DEPTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNPACK_COMPRESSED_BLOCK_SIZE");
//		lua_pushinteger(lua, GL_UNPACK_COMPRESSED_BLOCK_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PACK_COMPRESSED_BLOCK_WIDTH");
//		lua_pushinteger(lua, GL_PACK_COMPRESSED_BLOCK_WIDTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PACK_COMPRESSED_BLOCK_HEIGHT");
//		lua_pushinteger(lua, GL_PACK_COMPRESSED_BLOCK_HEIGHT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PACK_COMPRESSED_BLOCK_DEPTH");
//		lua_pushinteger(lua, GL_PACK_COMPRESSED_BLOCK_DEPTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PACK_COMPRESSED_BLOCK_SIZE");
//		lua_pushinteger(lua, GL_PACK_COMPRESSED_BLOCK_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_compute_shader");
//		lua_pushinteger(lua, GL_ARB_compute_shader);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_SHADER_BIT");
//		lua_pushinteger(lua, GL_COMPUTE_SHADER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_SHARED_MEMORY_SIZE");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_UNIFORM_COMPONENTS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_UNIFORM_COMPONENTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_WORK_GROUP_SIZE");
//		lua_pushinteger(lua, GL_COMPUTE_WORK_GROUP_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_WORK_GROUP_INVOCATIONS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER");
//		lua_pushinteger(lua, GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DISPATCH_INDIRECT_BUFFER");
//		lua_pushinteger(lua, GL_DISPATCH_INDIRECT_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DISPATCH_INDIRECT_BUFFER_BINDING");
//		lua_pushinteger(lua, GL_DISPATCH_INDIRECT_BUFFER_BINDING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_SHADER");
//		lua_pushinteger(lua, GL_COMPUTE_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_UNIFORM_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_UNIFORM_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_TEXTURE_IMAGE_UNITS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_WORK_GROUP_COUNT");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_WORK_GROUP_COUNT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_WORK_GROUP_SIZE");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_WORK_GROUP_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_compute_variable_group_size");
//		lua_pushinteger(lua, GL_ARB_compute_variable_group_size);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_FIXED_GROUP_INVOCATIONS_ARB");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_FIXED_GROUP_SIZE_ARB");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_conditional_render_inverted");
//		lua_pushinteger(lua, GL_ARB_conditional_render_inverted);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_WAIT_INVERTED");
//		lua_pushinteger(lua, GL_QUERY_WAIT_INVERTED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_NO_WAIT_INVERTED");
//		lua_pushinteger(lua, GL_QUERY_NO_WAIT_INVERTED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_BY_REGION_WAIT_INVERTED");
//		lua_pushinteger(lua, GL_QUERY_BY_REGION_WAIT_INVERTED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_BY_REGION_NO_WAIT_INVERTED");
//		lua_pushinteger(lua, GL_QUERY_BY_REGION_NO_WAIT_INVERTED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_conservative_depth");
//		lua_pushinteger(lua, GL_ARB_conservative_depth);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_copy_buffer");
		lua_pushinteger(lua, GL_ARB_copy_buffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY_READ_BUFFER");
		lua_pushinteger(lua, GL_COPY_READ_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COPY_WRITE_BUFFER");
		lua_pushinteger(lua, GL_COPY_WRITE_BUFFER);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_copy_image");
//		lua_pushinteger(lua, GL_ARB_copy_image);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_cull_distance");
//		lua_pushinteger(lua, GL_ARB_cull_distance);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_CULL_DISTANCES");
//		lua_pushinteger(lua, GL_MAX_CULL_DISTANCES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_CLIP_AND_CULL_DISTANCES");
//		lua_pushinteger(lua, GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_debug_output");
		lua_pushinteger(lua, GL_ARB_debug_output);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_OUTPUT_SYNCHRONOUS_ARB");
		lua_pushinteger(lua, GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB");
		lua_pushinteger(lua, GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CALLBACK_FUNCTION_ARB");
		lua_pushinteger(lua, GL_DEBUG_CALLBACK_FUNCTION_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_CALLBACK_USER_PARAM_ARB");
		lua_pushinteger(lua, GL_DEBUG_CALLBACK_USER_PARAM_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SOURCE_API_ARB");
		lua_pushinteger(lua, GL_DEBUG_SOURCE_API_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SOURCE_WINDOW_SYSTEM_ARB");
		lua_pushinteger(lua, GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SOURCE_SHADER_COMPILER_ARB");
		lua_pushinteger(lua, GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SOURCE_THIRD_PARTY_ARB");
		lua_pushinteger(lua, GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SOURCE_APPLICATION_ARB");
		lua_pushinteger(lua, GL_DEBUG_SOURCE_APPLICATION_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SOURCE_OTHER_ARB");
		lua_pushinteger(lua, GL_DEBUG_SOURCE_OTHER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_TYPE_ERROR_ARB");
		lua_pushinteger(lua, GL_DEBUG_TYPE_ERROR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB");
		lua_pushinteger(lua, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB");
		lua_pushinteger(lua, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_TYPE_PORTABILITY_ARB");
		lua_pushinteger(lua, GL_DEBUG_TYPE_PORTABILITY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_TYPE_PERFORMANCE_ARB");
		lua_pushinteger(lua, GL_DEBUG_TYPE_PERFORMANCE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_TYPE_OTHER_ARB");
		lua_pushinteger(lua, GL_DEBUG_TYPE_OTHER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DEBUG_MESSAGE_LENGTH_ARB");
		lua_pushinteger(lua, GL_MAX_DEBUG_MESSAGE_LENGTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DEBUG_LOGGED_MESSAGES_ARB");
		lua_pushinteger(lua, GL_MAX_DEBUG_LOGGED_MESSAGES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_LOGGED_MESSAGES_ARB");
		lua_pushinteger(lua, GL_DEBUG_LOGGED_MESSAGES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SEVERITY_HIGH_ARB");
		lua_pushinteger(lua, GL_DEBUG_SEVERITY_HIGH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SEVERITY_MEDIUM_ARB");
		lua_pushinteger(lua, GL_DEBUG_SEVERITY_MEDIUM_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEBUG_SEVERITY_LOW_ARB");
		lua_pushinteger(lua, GL_DEBUG_SEVERITY_LOW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_depth_buffer_float");
		lua_pushinteger(lua, GL_ARB_depth_buffer_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT32F");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT32F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH32F_STENCIL8");
		lua_pushinteger(lua, GL_DEPTH32F_STENCIL8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_32_UNSIGNED_INT_24_8_REV");
		lua_pushinteger(lua, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_depth_clamp");
		lua_pushinteger(lua, GL_ARB_depth_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_CLAMP");
		lua_pushinteger(lua, GL_DEPTH_CLAMP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_depth_texture");
		lua_pushinteger(lua, GL_ARB_depth_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT16_ARB");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT16_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT24_ARB");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT24_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT32_ARB");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT32_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH_SIZE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_TEXTURE_MODE_ARB");
		lua_pushinteger(lua, GL_DEPTH_TEXTURE_MODE_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_derivative_control");
//		lua_pushinteger(lua, GL_ARB_derivative_control);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_direct_state_access");
//		lua_pushinteger(lua, GL_ARB_direct_state_access);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_TARGET");
//		lua_pushinteger(lua, GL_TEXTURE_TARGET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_TARGET");
//		lua_pushinteger(lua, GL_QUERY_TARGET);
//		lua_settable(lua, -3);
		// lua_pushstring(lua, "TEXTURE_BINDING");
		// lua_pushinteger(lua, GL_TEXTURE_BINDING);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_draw_buffers");
		lua_pushinteger(lua, GL_ARB_draw_buffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DRAW_BUFFERS_ARB");
		lua_pushinteger(lua, GL_MAX_DRAW_BUFFERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER0_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER0_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER1_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER1_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER2_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER3_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER4_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER5_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER5_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER6_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER6_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER7_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER7_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER8_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER8_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER9_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER9_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER10_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER10_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER11_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER11_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER12_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER12_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER13_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER13_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER14_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER14_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER15_ARB");
		lua_pushinteger(lua, GL_DRAW_BUFFER15_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_draw_buffers_blend");
		lua_pushinteger(lua, GL_ARB_draw_buffers_blend);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_draw_elements_base_vertex");
		lua_pushinteger(lua, GL_ARB_draw_elements_base_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_draw_indirect");
		lua_pushinteger(lua, GL_ARB_draw_indirect);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_INDIRECT_BUFFER");
		lua_pushinteger(lua, GL_DRAW_INDIRECT_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_INDIRECT_BUFFER_BINDING");
		lua_pushinteger(lua, GL_DRAW_INDIRECT_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_draw_instanced");
		lua_pushinteger(lua, GL_ARB_draw_instanced);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_enhanced_layouts");
//		lua_pushinteger(lua, GL_ARB_enhanced_layouts);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOCATION_COMPONENT");
//		lua_pushinteger(lua, GL_LOCATION_COMPONENT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_INDEX");
//		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_INDEX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_STRIDE");
//		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_explicit_attrib_location");
		lua_pushinteger(lua, GL_ARB_explicit_attrib_location);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_explicit_uniform_location");
//		lua_pushinteger(lua, GL_ARB_explicit_uniform_location);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_UNIFORM_LOCATIONS");
//		lua_pushinteger(lua, GL_MAX_UNIFORM_LOCATIONS);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_fragment_coord_conventions");
		lua_pushinteger(lua, GL_ARB_fragment_coord_conventions);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_fragment_layer_viewport");
//		lua_pushinteger(lua, GL_ARB_fragment_layer_viewport);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_fragment_program");
		lua_pushinteger(lua, GL_ARB_fragment_program);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_PROGRAM_ARB");
		lua_pushinteger(lua, GL_FRAGMENT_PROGRAM_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ALU_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_ALU_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_TEX_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_TEX_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_TEX_INDIRECTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_TEX_INDIRECTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_ALU_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEX_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEX_INDIRECTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_COORDS_ARB");
		lua_pushinteger(lua, GL_MAX_TEXTURE_COORDS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_IMAGE_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_TEXTURE_IMAGE_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_fragment_program_shadow");
		lua_pushinteger(lua, GL_ARB_fragment_program_shadow);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_fragment_shader");
		lua_pushinteger(lua, GL_ARB_fragment_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_SHADER_ARB");
		lua_pushinteger(lua, GL_FRAGMENT_SHADER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_SHADER_DERIVATIVE_HINT_ARB");
		lua_pushinteger(lua, GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_framebuffer_no_attachments");
//		lua_pushinteger(lua, GL_ARB_framebuffer_no_attachments);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_DEFAULT_WIDTH");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_DEFAULT_WIDTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_DEFAULT_HEIGHT");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_DEFAULT_HEIGHT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_DEFAULT_LAYERS");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_DEFAULT_LAYERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_DEFAULT_SAMPLES");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_DEFAULT_SAMPLES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAMEBUFFER_WIDTH");
//		lua_pushinteger(lua, GL_MAX_FRAMEBUFFER_WIDTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAMEBUFFER_HEIGHT");
//		lua_pushinteger(lua, GL_MAX_FRAMEBUFFER_HEIGHT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAMEBUFFER_LAYERS");
//		lua_pushinteger(lua, GL_MAX_FRAMEBUFFER_LAYERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAMEBUFFER_SAMPLES");
//		lua_pushinteger(lua, GL_MAX_FRAMEBUFFER_SAMPLES);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_framebuffer_object");
		lua_pushinteger(lua, GL_ARB_framebuffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_FRAMEBUFFER_OPERATION");
		lua_pushinteger(lua, GL_INVALID_FRAMEBUFFER_OPERATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_RED_SIZE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_GREEN_SIZE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_BLUE_SIZE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_DEFAULT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_DEFAULT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_UNDEFINED");
		lua_pushinteger(lua, GL_FRAMEBUFFER_UNDEFINED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_STENCIL_ATTACHMENT");
		lua_pushinteger(lua, GL_DEPTH_STENCIL_ATTACHMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX");
		lua_pushinteger(lua, GL_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_RENDERBUFFER_SIZE");
		lua_pushinteger(lua, GL_MAX_RENDERBUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_STENCIL");
		lua_pushinteger(lua, GL_DEPTH_STENCIL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_24_8");
		lua_pushinteger(lua, GL_UNSIGNED_INT_24_8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH24_STENCIL8");
		lua_pushinteger(lua, GL_DEPTH24_STENCIL8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_STENCIL_SIZE");
		lua_pushinteger(lua, GL_TEXTURE_STENCIL_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_NORMALIZED");
		lua_pushinteger(lua, GL_UNSIGNED_NORMALIZED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB");
		lua_pushinteger(lua, GL_SRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_FRAMEBUFFER_BINDING");
		lua_pushinteger(lua, GL_DRAW_FRAMEBUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_BINDING");
		lua_pushinteger(lua, GL_FRAMEBUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_BINDING");
		lua_pushinteger(lua, GL_RENDERBUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_FRAMEBUFFER");
		lua_pushinteger(lua, GL_READ_FRAMEBUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_FRAMEBUFFER");
		lua_pushinteger(lua, GL_DRAW_FRAMEBUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_FRAMEBUFFER_BINDING");
		lua_pushinteger(lua, GL_READ_FRAMEBUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_SAMPLES");
		lua_pushinteger(lua, GL_RENDERBUFFER_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_COMPLETE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_COMPLETE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_UNSUPPORTED");
		lua_pushinteger(lua, GL_FRAMEBUFFER_UNSUPPORTED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COLOR_ATTACHMENTS");
		lua_pushinteger(lua, GL_MAX_COLOR_ATTACHMENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT0");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT0);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT1");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT2");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT3");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT4");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT5");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT6");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT6);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT7");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT7);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT8");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT9");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT9);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT10");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT10);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT11");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT11);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT12");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT12);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT13");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT13);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT14");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT14);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT15");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT15);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_ATTACHMENT");
		lua_pushinteger(lua, GL_DEPTH_ATTACHMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_ATTACHMENT");
		lua_pushinteger(lua, GL_STENCIL_ATTACHMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER");
		lua_pushinteger(lua, GL_FRAMEBUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER");
		lua_pushinteger(lua, GL_RENDERBUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_WIDTH");
		lua_pushinteger(lua, GL_RENDERBUFFER_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_HEIGHT");
		lua_pushinteger(lua, GL_RENDERBUFFER_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_INTERNAL_FORMAT");
		lua_pushinteger(lua, GL_RENDERBUFFER_INTERNAL_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX1");
		lua_pushinteger(lua, GL_STENCIL_INDEX1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX4");
		lua_pushinteger(lua, GL_STENCIL_INDEX4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX8");
		lua_pushinteger(lua, GL_STENCIL_INDEX8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX16");
		lua_pushinteger(lua, GL_STENCIL_INDEX16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_RED_SIZE");
		lua_pushinteger(lua, GL_RENDERBUFFER_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_GREEN_SIZE");
		lua_pushinteger(lua, GL_RENDERBUFFER_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_BLUE_SIZE");
		lua_pushinteger(lua, GL_RENDERBUFFER_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_ALPHA_SIZE");
		lua_pushinteger(lua, GL_RENDERBUFFER_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_DEPTH_SIZE");
		lua_pushinteger(lua, GL_RENDERBUFFER_DEPTH_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_STENCIL_SIZE");
		lua_pushinteger(lua, GL_RENDERBUFFER_STENCIL_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SAMPLES");
		lua_pushinteger(lua, GL_MAX_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_framebuffer_sRGB");
		lua_pushinteger(lua, GL_ARB_framebuffer_sRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_SRGB");
		lua_pushinteger(lua, GL_FRAMEBUFFER_SRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_geometry_shader4");
		lua_pushinteger(lua, GL_ARB_geometry_shader4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINES_ADJACENCY_ARB");
		lua_pushinteger(lua, GL_LINES_ADJACENCY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STRIP_ADJACENCY_ARB");
		lua_pushinteger(lua, GL_LINE_STRIP_ADJACENCY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLES_ADJACENCY_ARB");
		lua_pushinteger(lua, GL_TRIANGLES_ADJACENCY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_STRIP_ADJACENCY_ARB");
		lua_pushinteger(lua, GL_TRIANGLE_STRIP_ADJACENCY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_POINT_SIZE_ARB");
		lua_pushinteger(lua, GL_PROGRAM_POINT_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_LAYERED_ARB");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_SHADER_ARB");
		lua_pushinteger(lua, GL_GEOMETRY_SHADER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_VERTICES_OUT_ARB");
		lua_pushinteger(lua, GL_GEOMETRY_VERTICES_OUT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_INPUT_TYPE_ARB");
		lua_pushinteger(lua, GL_GEOMETRY_INPUT_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_OUTPUT_TYPE_ARB");
		lua_pushinteger(lua, GL_GEOMETRY_OUTPUT_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_VARYING_COMPONENTS_ARB");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_VARYING_COMPONENTS_ARB");
		lua_pushinteger(lua, GL_MAX_VERTEX_VARYING_COMPONENTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_OUTPUT_VERTICES_ARB");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_get_program_binary");
		lua_pushinteger(lua, GL_ARB_get_program_binary);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_BINARY_RETRIEVABLE_HINT");
		lua_pushinteger(lua, GL_PROGRAM_BINARY_RETRIEVABLE_HINT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_BINARY_LENGTH");
		lua_pushinteger(lua, GL_PROGRAM_BINARY_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_PROGRAM_BINARY_FORMATS");
		lua_pushinteger(lua, GL_NUM_PROGRAM_BINARY_FORMATS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_BINARY_FORMATS");
		lua_pushinteger(lua, GL_PROGRAM_BINARY_FORMATS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_get_texture_sub_image");
//		lua_pushinteger(lua, GL_ARB_get_texture_sub_image);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_gpu_shader5");
		lua_pushinteger(lua, GL_ARB_gpu_shader5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_SHADER_INVOCATIONS");
		lua_pushinteger(lua, GL_GEOMETRY_SHADER_INVOCATIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_SHADER_INVOCATIONS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_SHADER_INVOCATIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_FRAGMENT_INTERPOLATION_OFFSET");
		lua_pushinteger(lua, GL_MIN_FRAGMENT_INTERPOLATION_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_INTERPOLATION_OFFSET");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_INTERPOLATION_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_INTERPOLATION_OFFSET_BITS");
		lua_pushinteger(lua, GL_FRAGMENT_INTERPOLATION_OFFSET_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_STREAMS");
		lua_pushinteger(lua, GL_MAX_VERTEX_STREAMS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_gpu_shader_fp64");
		lua_pushinteger(lua, GL_ARB_gpu_shader_fp64);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT2");
		lua_pushinteger(lua, GL_DOUBLE_MAT2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT3");
		lua_pushinteger(lua, GL_DOUBLE_MAT3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT4");
		lua_pushinteger(lua, GL_DOUBLE_MAT4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT2x3");
		lua_pushinteger(lua, GL_DOUBLE_MAT2x3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT2x4");
		lua_pushinteger(lua, GL_DOUBLE_MAT2x4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT3x2");
		lua_pushinteger(lua, GL_DOUBLE_MAT3x2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT3x4");
		lua_pushinteger(lua, GL_DOUBLE_MAT3x4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT4x2");
		lua_pushinteger(lua, GL_DOUBLE_MAT4x2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT4x3");
		lua_pushinteger(lua, GL_DOUBLE_MAT4x3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_VEC2");
		lua_pushinteger(lua, GL_DOUBLE_VEC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_VEC3");
		lua_pushinteger(lua, GL_DOUBLE_VEC3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_VEC4");
		lua_pushinteger(lua, GL_DOUBLE_VEC4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_half_float_pixel");
		lua_pushinteger(lua, GL_ARB_half_float_pixel);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_FLOAT_ARB");
		lua_pushinteger(lua, GL_HALF_FLOAT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_half_float_vertex");
		lua_pushinteger(lua, GL_ARB_half_float_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_FLOAT");
		lua_pushinteger(lua, GL_HALF_FLOAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_imaging");
		lua_pushinteger(lua, GL_ARB_imaging);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_COLOR");
		lua_pushinteger(lua, GL_CONSTANT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_CONSTANT_COLOR");
		lua_pushinteger(lua, GL_ONE_MINUS_CONSTANT_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_ALPHA");
		lua_pushinteger(lua, GL_CONSTANT_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_CONSTANT_ALPHA");
		lua_pushinteger(lua, GL_ONE_MINUS_CONSTANT_ALPHA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_COLOR");
		lua_pushinteger(lua, GL_BLEND_COLOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_ADD");
		lua_pushinteger(lua, GL_FUNC_ADD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN");
		lua_pushinteger(lua, GL_MIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX");
		lua_pushinteger(lua, GL_MAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION");
		lua_pushinteger(lua, GL_BLEND_EQUATION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_SUBTRACT");
		lua_pushinteger(lua, GL_FUNC_SUBTRACT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_REVERSE_SUBTRACT");
		lua_pushinteger(lua, GL_FUNC_REVERSE_SUBTRACT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_1D");
		lua_pushinteger(lua, GL_CONVOLUTION_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_2D");
		lua_pushinteger(lua, GL_CONVOLUTION_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARABLE_2D");
		lua_pushinteger(lua, GL_SEPARABLE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_BORDER_MODE");
		lua_pushinteger(lua, GL_CONVOLUTION_BORDER_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FILTER_SCALE");
		lua_pushinteger(lua, GL_CONVOLUTION_FILTER_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FILTER_BIAS");
		lua_pushinteger(lua, GL_CONVOLUTION_FILTER_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REDUCE");
		lua_pushinteger(lua, GL_REDUCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FORMAT");
		lua_pushinteger(lua, GL_CONVOLUTION_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_WIDTH");
		lua_pushinteger(lua, GL_CONVOLUTION_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_HEIGHT");
		lua_pushinteger(lua, GL_CONVOLUTION_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CONVOLUTION_WIDTH");
		lua_pushinteger(lua, GL_MAX_CONVOLUTION_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CONVOLUTION_HEIGHT");
		lua_pushinteger(lua, GL_MAX_CONVOLUTION_HEIGHT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_RED_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_RED_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_GREEN_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_GREEN_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_BLUE_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_BLUE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_ALPHA_SCALE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_ALPHA_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_RED_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_RED_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_GREEN_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_GREEN_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_BLUE_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_BLUE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_ALPHA_BIAS");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_ALPHA_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM");
		lua_pushinteger(lua, GL_HISTOGRAM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_HISTOGRAM");
		lua_pushinteger(lua, GL_PROXY_HISTOGRAM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_WIDTH");
		lua_pushinteger(lua, GL_HISTOGRAM_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_FORMAT");
		lua_pushinteger(lua, GL_HISTOGRAM_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_RED_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_GREEN_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_BLUE_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_ALPHA_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_LUMINANCE_SIZE");
		lua_pushinteger(lua, GL_HISTOGRAM_LUMINANCE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_SINK");
		lua_pushinteger(lua, GL_HISTOGRAM_SINK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX");
		lua_pushinteger(lua, GL_MINMAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_FORMAT");
		lua_pushinteger(lua, GL_MINMAX_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_SINK");
		lua_pushinteger(lua, GL_MINMAX_SINK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TABLE_TOO_LARGE");
		lua_pushinteger(lua, GL_TABLE_TOO_LARGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATRIX");
		lua_pushinteger(lua, GL_COLOR_MATRIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATRIX_STACK_DEPTH");
		lua_pushinteger(lua, GL_COLOR_MATRIX_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COLOR_MATRIX_STACK_DEPTH");
		lua_pushinteger(lua, GL_MAX_COLOR_MATRIX_STACK_DEPTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_RED_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_RED_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_GREEN_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_GREEN_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_BLUE_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_BLUE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_ALPHA_SCALE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_ALPHA_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_RED_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_RED_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_GREEN_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_GREEN_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_BLUE_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_BLUE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_ALPHA_BIAS");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_ALPHA_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE");
		lua_pushinteger(lua, GL_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_COLOR_TABLE");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_COLOR_TABLE");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_COLOR_TABLE");
		lua_pushinteger(lua, GL_PROXY_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_POST_CONVOLUTION_COLOR_TABLE");
		lua_pushinteger(lua, GL_PROXY_POST_CONVOLUTION_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_POST_COLOR_MATRIX_COLOR_TABLE");
		lua_pushinteger(lua, GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_SCALE");
		lua_pushinteger(lua, GL_COLOR_TABLE_SCALE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_BIAS");
		lua_pushinteger(lua, GL_COLOR_TABLE_BIAS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_FORMAT");
		lua_pushinteger(lua, GL_COLOR_TABLE_FORMAT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_WIDTH");
		lua_pushinteger(lua, GL_COLOR_TABLE_WIDTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_RED_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_RED_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_GREEN_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_GREEN_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_BLUE_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_BLUE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_ALPHA_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_ALPHA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_LUMINANCE_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_LUMINANCE_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_INTENSITY_SIZE");
		lua_pushinteger(lua, GL_COLOR_TABLE_INTENSITY_SIZE);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "IGNORE_BORDER");
//		lua_pushinteger(lua, GL_IGNORE_BORDER);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_BORDER");
		lua_pushinteger(lua, GL_CONSTANT_BORDER);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "WRAP_BORDER");
//		lua_pushinteger(lua, GL_WRAP_BORDER);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLICATE_BORDER");
		lua_pushinteger(lua, GL_REPLICATE_BORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_BORDER_COLOR");
		lua_pushinteger(lua, GL_CONVOLUTION_BORDER_COLOR);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_indirect_parameters");
//		lua_pushinteger(lua, GL_ARB_indirect_parameters);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PARAMETER_BUFFER_ARB");
//		lua_pushinteger(lua, GL_PARAMETER_BUFFER_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PARAMETER_BUFFER_BINDING_ARB");
//		lua_pushinteger(lua, GL_PARAMETER_BUFFER_BINDING_ARB);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_instanced_arrays");
		lua_pushinteger(lua, GL_ARB_instanced_arrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_DIVISOR_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_internalformat_query");
//		lua_pushinteger(lua, GL_ARB_internalformat_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NUM_SAMPLE_COUNTS");
//		lua_pushinteger(lua, GL_NUM_SAMPLE_COUNTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_internalformat_query2");
//		lua_pushinteger(lua, GL_ARB_internalformat_query2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_SUPPORTED");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_SUPPORTED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_PREFERRED");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_PREFERRED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_RED_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_RED_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_GREEN_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_GREEN_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_BLUE_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_BLUE_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_ALPHA_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_ALPHA_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_DEPTH_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_DEPTH_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_STENCIL_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_STENCIL_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_SHARED_SIZE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_SHARED_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_RED_TYPE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_RED_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_GREEN_TYPE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_GREEN_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_BLUE_TYPE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_BLUE_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_ALPHA_TYPE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_ALPHA_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_DEPTH_TYPE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_DEPTH_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTERNALFORMAT_STENCIL_TYPE");
//		lua_pushinteger(lua, GL_INTERNALFORMAT_STENCIL_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_WIDTH");
//		lua_pushinteger(lua, GL_MAX_WIDTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_HEIGHT");
//		lua_pushinteger(lua, GL_MAX_HEIGHT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_DEPTH");
//		lua_pushinteger(lua, GL_MAX_DEPTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_LAYERS");
//		lua_pushinteger(lua, GL_MAX_LAYERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_DIMENSIONS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_DIMENSIONS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_COMPONENTS");
//		lua_pushinteger(lua, GL_COLOR_COMPONENTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEPTH_COMPONENTS");
//		lua_pushinteger(lua, GL_DEPTH_COMPONENTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STENCIL_COMPONENTS");
//		lua_pushinteger(lua, GL_STENCIL_COMPONENTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_RENDERABLE");
//		lua_pushinteger(lua, GL_COLOR_RENDERABLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEPTH_RENDERABLE");
//		lua_pushinteger(lua, GL_DEPTH_RENDERABLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STENCIL_RENDERABLE");
//		lua_pushinteger(lua, GL_STENCIL_RENDERABLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_RENDERABLE");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_RENDERABLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_RENDERABLE_LAYERED");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_RENDERABLE_LAYERED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_BLEND");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_BLEND);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "READ_PIXELS");
//		lua_pushinteger(lua, GL_READ_PIXELS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "READ_PIXELS_FORMAT");
//		lua_pushinteger(lua, GL_READ_PIXELS_FORMAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "READ_PIXELS_TYPE");
//		lua_pushinteger(lua, GL_READ_PIXELS_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_IMAGE_FORMAT");
//		lua_pushinteger(lua, GL_TEXTURE_IMAGE_FORMAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_IMAGE_TYPE");
//		lua_pushinteger(lua, GL_TEXTURE_IMAGE_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GET_TEXTURE_IMAGE_FORMAT");
//		lua_pushinteger(lua, GL_GET_TEXTURE_IMAGE_FORMAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GET_TEXTURE_IMAGE_TYPE");
//		lua_pushinteger(lua, GL_GET_TEXTURE_IMAGE_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIPMAP");
//		lua_pushinteger(lua, GL_MIPMAP);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MANUAL_GENERATE_MIPMAP");
//		lua_pushinteger(lua, GL_MANUAL_GENERATE_MIPMAP);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AUTO_GENERATE_MIPMAP");
//		lua_pushinteger(lua, GL_AUTO_GENERATE_MIPMAP);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_ENCODING");
//		lua_pushinteger(lua, GL_COLOR_ENCODING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRGB_READ");
//		lua_pushinteger(lua, GL_SRGB_READ);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRGB_WRITE");
//		lua_pushinteger(lua, GL_SRGB_WRITE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRGB_DECODE_ARB");
//		lua_pushinteger(lua, GL_SRGB_DECODE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FILTER");
//		lua_pushinteger(lua, GL_FILTER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_TEXTURE");
//		lua_pushinteger(lua, GL_VERTEX_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_CONTROL_TEXTURE");
//		lua_pushinteger(lua, GL_TESS_CONTROL_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_EVALUATION_TEXTURE");
//		lua_pushinteger(lua, GL_TESS_EVALUATION_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GEOMETRY_TEXTURE");
//		lua_pushinteger(lua, GL_GEOMETRY_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_TEXTURE");
//		lua_pushinteger(lua, GL_FRAGMENT_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_TEXTURE");
//		lua_pushinteger(lua, GL_COMPUTE_TEXTURE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_SHADOW");
//		lua_pushinteger(lua, GL_TEXTURE_SHADOW);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_GATHER");
//		lua_pushinteger(lua, GL_TEXTURE_GATHER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_GATHER_SHADOW");
//		lua_pushinteger(lua, GL_TEXTURE_GATHER_SHADOW);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_IMAGE_LOAD");
//		lua_pushinteger(lua, GL_SHADER_IMAGE_LOAD);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_IMAGE_STORE");
//		lua_pushinteger(lua, GL_SHADER_IMAGE_STORE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_IMAGE_ATOMIC");
//		lua_pushinteger(lua, GL_SHADER_IMAGE_ATOMIC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_TEXEL_SIZE");
//		lua_pushinteger(lua, GL_IMAGE_TEXEL_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_COMPATIBILITY_CLASS");
//		lua_pushinteger(lua, GL_IMAGE_COMPATIBILITY_CLASS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_PIXEL_FORMAT");
//		lua_pushinteger(lua, GL_IMAGE_PIXEL_FORMAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_PIXEL_TYPE");
//		lua_pushinteger(lua, GL_IMAGE_PIXEL_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST");
//		lua_pushinteger(lua, GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST");
//		lua_pushinteger(lua, GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE");
//		lua_pushinteger(lua, GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE");
//		lua_pushinteger(lua, GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_COMPRESSED_BLOCK_WIDTH");
//		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_BLOCK_WIDTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_COMPRESSED_BLOCK_HEIGHT");
//		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_COMPRESSED_BLOCK_SIZE");
//		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_BLOCK_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLEAR_BUFFER");
//		lua_pushinteger(lua, GL_CLEAR_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_VIEW");
//		lua_pushinteger(lua, GL_TEXTURE_VIEW);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_COMPATIBILITY_CLASS");
//		lua_pushinteger(lua, GL_VIEW_COMPATIBILITY_CLASS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FULL_SUPPORT");
//		lua_pushinteger(lua, GL_FULL_SUPPORT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CAVEAT_SUPPORT");
//		lua_pushinteger(lua, GL_CAVEAT_SUPPORT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_4_X_32");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_4_X_32);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_2_X_32");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_2_X_32);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_1_X_32");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_1_X_32);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_4_X_16");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_4_X_16);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_2_X_16");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_2_X_16);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_1_X_16");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_1_X_16);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_4_X_8");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_4_X_8);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_2_X_8");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_2_X_8);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_1_X_8");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_1_X_8);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_11_11_10");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_11_11_10);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CLASS_10_10_10_2");
//		lua_pushinteger(lua, GL_IMAGE_CLASS_10_10_10_2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_128_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_128_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_96_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_96_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_64_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_64_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_48_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_48_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_32_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_32_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_24_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_24_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_16_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_16_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_8_BITS");
//		lua_pushinteger(lua, GL_VIEW_CLASS_8_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_S3TC_DXT1_RGB");
//		lua_pushinteger(lua, GL_VIEW_CLASS_S3TC_DXT1_RGB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_S3TC_DXT1_RGBA");
//		lua_pushinteger(lua, GL_VIEW_CLASS_S3TC_DXT1_RGBA);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_S3TC_DXT3_RGBA");
//		lua_pushinteger(lua, GL_VIEW_CLASS_S3TC_DXT3_RGBA);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_S3TC_DXT5_RGBA");
//		lua_pushinteger(lua, GL_VIEW_CLASS_S3TC_DXT5_RGBA);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_RGTC1_RED");
//		lua_pushinteger(lua, GL_VIEW_CLASS_RGTC1_RED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_RGTC2_RG");
//		lua_pushinteger(lua, GL_VIEW_CLASS_RGTC2_RG);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_BPTC_UNORM");
//		lua_pushinteger(lua, GL_VIEW_CLASS_BPTC_UNORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIEW_CLASS_BPTC_FLOAT");
//		lua_pushinteger(lua, GL_VIEW_CLASS_BPTC_FLOAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_invalidate_subdata");
//		lua_pushinteger(lua, GL_ARB_invalidate_subdata);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_map_buffer_alignment");
//		lua_pushinteger(lua, GL_ARB_map_buffer_alignment);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_MAP_BUFFER_ALIGNMENT");
//		lua_pushinteger(lua, GL_MIN_MAP_BUFFER_ALIGNMENT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_map_buffer_range");
		lua_pushinteger(lua, GL_ARB_map_buffer_range);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_READ_BIT");
		lua_pushinteger(lua, GL_MAP_READ_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_WRITE_BIT");
		lua_pushinteger(lua, GL_MAP_WRITE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_INVALIDATE_RANGE_BIT");
		lua_pushinteger(lua, GL_MAP_INVALIDATE_RANGE_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_INVALIDATE_BUFFER_BIT");
		lua_pushinteger(lua, GL_MAP_INVALIDATE_BUFFER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_FLUSH_EXPLICIT_BIT");
		lua_pushinteger(lua, GL_MAP_FLUSH_EXPLICIT_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_UNSYNCHRONIZED_BIT");
		lua_pushinteger(lua, GL_MAP_UNSYNCHRONIZED_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_matrix_palette");
		lua_pushinteger(lua, GL_ARB_matrix_palette);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_PALETTE_ARB");
		lua_pushinteger(lua, GL_MATRIX_PALETTE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_MATRIX_PALETTE_STACK_DEPTH_ARB");
		lua_pushinteger(lua, GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PALETTE_MATRICES_ARB");
		lua_pushinteger(lua, GL_MAX_PALETTE_MATRICES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_PALETTE_MATRIX_ARB");
		lua_pushinteger(lua, GL_CURRENT_PALETTE_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_INDEX_ARRAY_ARB");
		lua_pushinteger(lua, GL_MATRIX_INDEX_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_MATRIX_INDEX_ARB");
		lua_pushinteger(lua, GL_CURRENT_MATRIX_INDEX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_INDEX_ARRAY_SIZE_ARB");
		lua_pushinteger(lua, GL_MATRIX_INDEX_ARRAY_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_INDEX_ARRAY_TYPE_ARB");
		lua_pushinteger(lua, GL_MATRIX_INDEX_ARRAY_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_INDEX_ARRAY_STRIDE_ARB");
		lua_pushinteger(lua, GL_MATRIX_INDEX_ARRAY_STRIDE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_INDEX_ARRAY_POINTER_ARB");
		lua_pushinteger(lua, GL_MATRIX_INDEX_ARRAY_POINTER_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_multi_bind");
//		lua_pushinteger(lua, GL_ARB_multi_bind);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_multi_draw_indirect");
//		lua_pushinteger(lua, GL_ARB_multi_draw_indirect);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_multisample");
		lua_pushinteger(lua, GL_ARB_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_ARB");
		lua_pushinteger(lua, GL_MULTISAMPLE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_COVERAGE_ARB");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_ONE_ARB");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_ONE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_ARB");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_BUFFERS_ARB");
		lua_pushinteger(lua, GL_SAMPLE_BUFFERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES_ARB");
		lua_pushinteger(lua, GL_SAMPLES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_VALUE_ARB");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_VALUE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_COVERAGE_INVERT_ARB");
		lua_pushinteger(lua, GL_SAMPLE_COVERAGE_INVERT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_BIT_ARB");
		lua_pushinteger(lua, GL_MULTISAMPLE_BIT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_multitexture");
		lua_pushinteger(lua, GL_ARB_multitexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE0_ARB");
		lua_pushinteger(lua, GL_TEXTURE0_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE1_ARB");
		lua_pushinteger(lua, GL_TEXTURE1_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE2_ARB");
		lua_pushinteger(lua, GL_TEXTURE2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE3_ARB");
		lua_pushinteger(lua, GL_TEXTURE3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE4_ARB");
		lua_pushinteger(lua, GL_TEXTURE4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE5_ARB");
		lua_pushinteger(lua, GL_TEXTURE5_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE6_ARB");
		lua_pushinteger(lua, GL_TEXTURE6_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE7_ARB");
		lua_pushinteger(lua, GL_TEXTURE7_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE8_ARB");
		lua_pushinteger(lua, GL_TEXTURE8_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE9_ARB");
		lua_pushinteger(lua, GL_TEXTURE9_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE10_ARB");
		lua_pushinteger(lua, GL_TEXTURE10_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE11_ARB");
		lua_pushinteger(lua, GL_TEXTURE11_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE12_ARB");
		lua_pushinteger(lua, GL_TEXTURE12_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE13_ARB");
		lua_pushinteger(lua, GL_TEXTURE13_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE14_ARB");
		lua_pushinteger(lua, GL_TEXTURE14_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE15_ARB");
		lua_pushinteger(lua, GL_TEXTURE15_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE16_ARB");
		lua_pushinteger(lua, GL_TEXTURE16_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE17_ARB");
		lua_pushinteger(lua, GL_TEXTURE17_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE18_ARB");
		lua_pushinteger(lua, GL_TEXTURE18_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE19_ARB");
		lua_pushinteger(lua, GL_TEXTURE19_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE20_ARB");
		lua_pushinteger(lua, GL_TEXTURE20_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE21_ARB");
		lua_pushinteger(lua, GL_TEXTURE21_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE22_ARB");
		lua_pushinteger(lua, GL_TEXTURE22_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE23_ARB");
		lua_pushinteger(lua, GL_TEXTURE23_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE24_ARB");
		lua_pushinteger(lua, GL_TEXTURE24_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE25_ARB");
		lua_pushinteger(lua, GL_TEXTURE25_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE26_ARB");
		lua_pushinteger(lua, GL_TEXTURE26_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE27_ARB");
		lua_pushinteger(lua, GL_TEXTURE27_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE28_ARB");
		lua_pushinteger(lua, GL_TEXTURE28_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE29_ARB");
		lua_pushinteger(lua, GL_TEXTURE29_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE30_ARB");
		lua_pushinteger(lua, GL_TEXTURE30_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE31_ARB");
		lua_pushinteger(lua, GL_TEXTURE31_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_TEXTURE_ARB");
		lua_pushinteger(lua, GL_ACTIVE_TEXTURE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIENT_ACTIVE_TEXTURE_ARB");
		lua_pushinteger(lua, GL_CLIENT_ACTIVE_TEXTURE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_TEXTURE_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_occlusion_query");
		lua_pushinteger(lua, GL_ARB_occlusion_query);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_COUNTER_BITS_ARB");
		lua_pushinteger(lua, GL_QUERY_COUNTER_BITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_QUERY_ARB");
		lua_pushinteger(lua, GL_CURRENT_QUERY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_RESULT_ARB");
		lua_pushinteger(lua, GL_QUERY_RESULT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_RESULT_AVAILABLE_ARB");
		lua_pushinteger(lua, GL_QUERY_RESULT_AVAILABLE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES_PASSED_ARB");
		lua_pushinteger(lua, GL_SAMPLES_PASSED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_occlusion_query2");
		lua_pushinteger(lua, GL_ARB_occlusion_query2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ANY_SAMPLES_PASSED");
		lua_pushinteger(lua, GL_ANY_SAMPLES_PASSED);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_pipeline_statistics_query");
//		lua_pushinteger(lua, GL_ARB_pipeline_statistics_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTICES_SUBMITTED_ARB");
//		lua_pushinteger(lua, GL_VERTICES_SUBMITTED_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PRIMITIVES_SUBMITTED_ARB");
//		lua_pushinteger(lua, GL_PRIMITIVES_SUBMITTED_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_SHADER_INVOCATIONS_ARB");
//		lua_pushinteger(lua, GL_VERTEX_SHADER_INVOCATIONS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_CONTROL_SHADER_PATCHES_ARB");
//		lua_pushinteger(lua, GL_TESS_CONTROL_SHADER_PATCHES_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_EVALUATION_SHADER_INVOCATIONS_ARB");
//		lua_pushinteger(lua, GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB");
//		lua_pushinteger(lua, GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_SHADER_INVOCATIONS_ARB");
//		lua_pushinteger(lua, GL_FRAGMENT_SHADER_INVOCATIONS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_SHADER_INVOCATIONS_ARB");
//		lua_pushinteger(lua, GL_COMPUTE_SHADER_INVOCATIONS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLIPPING_INPUT_PRIMITIVES_ARB");
//		lua_pushinteger(lua, GL_CLIPPING_INPUT_PRIMITIVES_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLIPPING_OUTPUT_PRIMITIVES_ARB");
//		lua_pushinteger(lua, GL_CLIPPING_OUTPUT_PRIMITIVES_ARB);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_SHADER_INVOCATIONS");
		lua_pushinteger(lua, GL_GEOMETRY_SHADER_INVOCATIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_pixel_buffer_object");
		lua_pushinteger(lua, GL_ARB_pixel_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_PACK_BUFFER_ARB");
		lua_pushinteger(lua, GL_PIXEL_PACK_BUFFER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_UNPACK_BUFFER_ARB");
		lua_pushinteger(lua, GL_PIXEL_UNPACK_BUFFER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_PACK_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_PIXEL_PACK_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_UNPACK_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_PIXEL_UNPACK_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_point_parameters");
		lua_pushinteger(lua, GL_ARB_point_parameters);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_MIN_ARB");
		lua_pushinteger(lua, GL_POINT_SIZE_MIN_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_MAX_ARB");
		lua_pushinteger(lua, GL_POINT_SIZE_MAX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_FADE_THRESHOLD_SIZE_ARB");
		lua_pushinteger(lua, GL_POINT_FADE_THRESHOLD_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_DISTANCE_ATTENUATION_ARB");
		lua_pushinteger(lua, GL_POINT_DISTANCE_ATTENUATION_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_point_sprite");
		lua_pushinteger(lua, GL_ARB_point_sprite);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SPRITE_ARB");
		lua_pushinteger(lua, GL_POINT_SPRITE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COORD_REPLACE_ARB");
		lua_pushinteger(lua, GL_COORD_REPLACE_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_program_interface_query");
//		lua_pushinteger(lua, GL_ARB_program_interface_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM");
//		lua_pushinteger(lua, GL_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_BLOCK");
//		lua_pushinteger(lua, GL_UNIFORM_BLOCK);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM_INPUT");
//		lua_pushinteger(lua, GL_PROGRAM_INPUT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM_OUTPUT");
//		lua_pushinteger(lua, GL_PROGRAM_OUTPUT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_VARIABLE");
//		lua_pushinteger(lua, GL_BUFFER_VARIABLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BLOCK");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BLOCK);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IS_PER_PATCH");
//		lua_pushinteger(lua, GL_IS_PER_PATCH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_SUBROUTINE");
//		lua_pushinteger(lua, GL_VERTEX_SUBROUTINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_CONTROL_SUBROUTINE");
//		lua_pushinteger(lua, GL_TESS_CONTROL_SUBROUTINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_EVALUATION_SUBROUTINE");
//		lua_pushinteger(lua, GL_TESS_EVALUATION_SUBROUTINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GEOMETRY_SUBROUTINE");
//		lua_pushinteger(lua, GL_GEOMETRY_SUBROUTINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_SUBROUTINE");
//		lua_pushinteger(lua, GL_FRAGMENT_SUBROUTINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_SUBROUTINE");
//		lua_pushinteger(lua, GL_COMPUTE_SUBROUTINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_SUBROUTINE_UNIFORM");
//		lua_pushinteger(lua, GL_VERTEX_SUBROUTINE_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_CONTROL_SUBROUTINE_UNIFORM");
//		lua_pushinteger(lua, GL_TESS_CONTROL_SUBROUTINE_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TESS_EVALUATION_SUBROUTINE_UNIFORM");
//		lua_pushinteger(lua, GL_TESS_EVALUATION_SUBROUTINE_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GEOMETRY_SUBROUTINE_UNIFORM");
//		lua_pushinteger(lua, GL_GEOMETRY_SUBROUTINE_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_SUBROUTINE_UNIFORM");
//		lua_pushinteger(lua, GL_FRAGMENT_SUBROUTINE_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_SUBROUTINE_UNIFORM");
//		lua_pushinteger(lua, GL_COMPUTE_SUBROUTINE_UNIFORM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSFORM_FEEDBACK_VARYING");
//		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_VARYING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ACTIVE_RESOURCES");
//		lua_pushinteger(lua, GL_ACTIVE_RESOURCES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_NAME_LENGTH");
//		lua_pushinteger(lua, GL_MAX_NAME_LENGTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_NUM_ACTIVE_VARIABLES");
//		lua_pushinteger(lua, GL_MAX_NUM_ACTIVE_VARIABLES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_NUM_COMPATIBLE_SUBROUTINES");
//		lua_pushinteger(lua, GL_MAX_NUM_COMPATIBLE_SUBROUTINES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NAME_LENGTH");
//		lua_pushinteger(lua, GL_NAME_LENGTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TYPE");
//		lua_pushinteger(lua, GL_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARRAY_SIZE");
//		lua_pushinteger(lua, GL_ARRAY_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "OFFSET");
//		lua_pushinteger(lua, GL_OFFSET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BLOCK_INDEX");
//		lua_pushinteger(lua, GL_BLOCK_INDEX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARRAY_STRIDE");
//		lua_pushinteger(lua, GL_ARRAY_STRIDE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MATRIX_STRIDE");
//		lua_pushinteger(lua, GL_MATRIX_STRIDE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IS_ROW_MAJOR");
//		lua_pushinteger(lua, GL_IS_ROW_MAJOR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_INDEX");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_INDEX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_BINDING");
//		lua_pushinteger(lua, GL_BUFFER_BINDING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_DATA_SIZE");
//		lua_pushinteger(lua, GL_BUFFER_DATA_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NUM_ACTIVE_VARIABLES");
//		lua_pushinteger(lua, GL_NUM_ACTIVE_VARIABLES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ACTIVE_VARIABLES");
//		lua_pushinteger(lua, GL_ACTIVE_VARIABLES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REFERENCED_BY_VERTEX_SHADER");
//		lua_pushinteger(lua, GL_REFERENCED_BY_VERTEX_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REFERENCED_BY_TESS_CONTROL_SHADER");
//		lua_pushinteger(lua, GL_REFERENCED_BY_TESS_CONTROL_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REFERENCED_BY_TESS_EVALUATION_SHADER");
//		lua_pushinteger(lua, GL_REFERENCED_BY_TESS_EVALUATION_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REFERENCED_BY_GEOMETRY_SHADER");
//		lua_pushinteger(lua, GL_REFERENCED_BY_GEOMETRY_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REFERENCED_BY_FRAGMENT_SHADER");
//		lua_pushinteger(lua, GL_REFERENCED_BY_FRAGMENT_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REFERENCED_BY_COMPUTE_SHADER");
//		lua_pushinteger(lua, GL_REFERENCED_BY_COMPUTE_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TOP_LEVEL_ARRAY_SIZE");
//		lua_pushinteger(lua, GL_TOP_LEVEL_ARRAY_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TOP_LEVEL_ARRAY_STRIDE");
//		lua_pushinteger(lua, GL_TOP_LEVEL_ARRAY_STRIDE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOCATION");
//		lua_pushinteger(lua, GL_LOCATION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOCATION_INDEX");
//		lua_pushinteger(lua, GL_LOCATION_INDEX);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_provoking_vertex");
		lua_pushinteger(lua, GL_ARB_provoking_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION");
		lua_pushinteger(lua, GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIRST_VERTEX_CONVENTION");
		lua_pushinteger(lua, GL_FIRST_VERTEX_CONVENTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LAST_VERTEX_CONVENTION");
		lua_pushinteger(lua, GL_LAST_VERTEX_CONVENTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROVOKING_VERTEX");
		lua_pushinteger(lua, GL_PROVOKING_VERTEX);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_query_buffer_object");
//		lua_pushinteger(lua, GL_ARB_query_buffer_object);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_BUFFER_BARRIER_BIT");
//		lua_pushinteger(lua, GL_QUERY_BUFFER_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_BUFFER");
//		lua_pushinteger(lua, GL_QUERY_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_BUFFER_BINDING");
//		lua_pushinteger(lua, GL_QUERY_BUFFER_BINDING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_RESULT_NO_WAIT");
//		lua_pushinteger(lua, GL_QUERY_RESULT_NO_WAIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_robust_buffer_access_behavior");
//		lua_pushinteger(lua, GL_ARB_robust_buffer_access_behavior);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_robustness");
		lua_pushinteger(lua, GL_ARB_robustness);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB");
		lua_pushinteger(lua, GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOSE_CONTEXT_ON_RESET_ARB");
		lua_pushinteger(lua, GL_LOSE_CONTEXT_ON_RESET_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GUILTY_CONTEXT_RESET_ARB");
		lua_pushinteger(lua, GL_GUILTY_CONTEXT_RESET_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INNOCENT_CONTEXT_RESET_ARB");
		lua_pushinteger(lua, GL_INNOCENT_CONTEXT_RESET_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNKNOWN_CONTEXT_RESET_ARB");
		lua_pushinteger(lua, GL_UNKNOWN_CONTEXT_RESET_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESET_NOTIFICATION_STRATEGY_ARB");
		lua_pushinteger(lua, GL_RESET_NOTIFICATION_STRATEGY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NO_RESET_NOTIFICATION_ARB");
		lua_pushinteger(lua, GL_NO_RESET_NOTIFICATION_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_robustness_application_isolation");
//		lua_pushinteger(lua, GL_ARB_robustness_application_isolation);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_robustness_share_group_isolation");
//		lua_pushinteger(lua, GL_ARB_robustness_share_group_isolation);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_sample_shading");
		lua_pushinteger(lua, GL_ARB_sample_shading);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_SHADING_ARB");
		lua_pushinteger(lua, GL_SAMPLE_SHADING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_SAMPLE_SHADING_VALUE_ARB");
		lua_pushinteger(lua, GL_MIN_SAMPLE_SHADING_VALUE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_sampler_objects");
		lua_pushinteger(lua, GL_ARB_sampler_objects);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_BINDING");
		lua_pushinteger(lua, GL_SAMPLER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_seamless_cube_map");
		lua_pushinteger(lua, GL_ARB_seamless_cube_map);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_SEAMLESS");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_SEAMLESS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_seamless_cubemap_per_texture");
//		lua_pushinteger(lua, GL_ARB_seamless_cubemap_per_texture);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_SEAMLESS");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_SEAMLESS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_separate_shader_objects");
		lua_pushinteger(lua, GL_ARB_separate_shader_objects);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_BIT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_SHADER_BIT");
		lua_pushinteger(lua, GL_FRAGMENT_SHADER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_SHADER_BIT");
		lua_pushinteger(lua, GL_GEOMETRY_SHADER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_CONTROL_SHADER_BIT");
		lua_pushinteger(lua, GL_TESS_CONTROL_SHADER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_EVALUATION_SHADER_BIT");
		lua_pushinteger(lua, GL_TESS_EVALUATION_SHADER_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_SEPARABLE");
		lua_pushinteger(lua, GL_PROGRAM_SEPARABLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_PROGRAM");
		lua_pushinteger(lua, GL_ACTIVE_PROGRAM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_PIPELINE_BINDING");
		lua_pushinteger(lua, GL_PROGRAM_PIPELINE_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALL_SHADER_BITS");
		lua_pushinteger(lua, GL_ALL_SHADER_BITS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_atomic_counters");
//		lua_pushinteger(lua, GL_ARB_shader_atomic_counters);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_BINDING");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_BINDING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_START");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_START);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_SIZE");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_DATA_SIZE");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_VERTEX_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_CONTROL_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_EVALUATION_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_GEOMETRY_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_GEOMETRY_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAGMENT_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_FRAGMENT_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_ATOMIC_COUNTERS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_ATOMIC_COUNTERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_ATOMIC_COUNTER_BUFFER_SIZE");
//		lua_pushinteger(lua, GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ACTIVE_ATOMIC_COUNTER_BUFFERS");
//		lua_pushinteger(lua, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX");
//		lua_pushinteger(lua, GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_ATOMIC_COUNTER");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_ATOMIC_COUNTER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_ATOMIC_COUNTER_BUFFER_BINDINGS");
//		lua_pushinteger(lua, GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_bit_encoding");
//		lua_pushinteger(lua, GL_ARB_shader_bit_encoding);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_draw_parameters");
//		lua_pushinteger(lua, GL_ARB_shader_draw_parameters);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_group_vote");
//		lua_pushinteger(lua, GL_ARB_shader_group_vote);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_image_load_store");
//		lua_pushinteger(lua, GL_ARB_shader_image_load_store);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_BARRIER_BIT");
//		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ELEMENT_ARRAY_BARRIER_BIT");
//		lua_pushinteger(lua, GL_ELEMENT_ARRAY_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_BARRIER_BIT");
//		lua_pushinteger(lua, GL_UNIFORM_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_FETCH_BARRIER_BIT");
//		lua_pushinteger(lua, GL_TEXTURE_FETCH_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_IMAGE_ACCESS_BARRIER_BIT");
//		lua_pushinteger(lua, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMMAND_BARRIER_BIT");
//		lua_pushinteger(lua, GL_COMMAND_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PIXEL_BUFFER_BARRIER_BIT");
//		lua_pushinteger(lua, GL_PIXEL_BUFFER_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_UPDATE_BARRIER_BIT");
//		lua_pushinteger(lua, GL_TEXTURE_UPDATE_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_UPDATE_BARRIER_BIT");
//		lua_pushinteger(lua, GL_BUFFER_UPDATE_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_BARRIER_BIT");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BARRIER_BIT");
//		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATOMIC_COUNTER_BARRIER_BIT");
//		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_IMAGE_UNITS");
//		lua_pushinteger(lua, GL_MAX_IMAGE_UNITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BINDING_NAME");
//		lua_pushinteger(lua, GL_IMAGE_BINDING_NAME);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BINDING_LEVEL");
//		lua_pushinteger(lua, GL_IMAGE_BINDING_LEVEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BINDING_LAYERED");
//		lua_pushinteger(lua, GL_IMAGE_BINDING_LAYERED);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BINDING_LAYER");
//		lua_pushinteger(lua, GL_IMAGE_BINDING_LAYER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BINDING_ACCESS");
//		lua_pushinteger(lua, GL_IMAGE_BINDING_ACCESS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_1D");
//		lua_pushinteger(lua, GL_IMAGE_1D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_2D");
//		lua_pushinteger(lua, GL_IMAGE_2D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_3D");
//		lua_pushinteger(lua, GL_IMAGE_3D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_2D_RECT");
//		lua_pushinteger(lua, GL_IMAGE_2D_RECT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CUBE");
//		lua_pushinteger(lua, GL_IMAGE_CUBE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BUFFER");
//		lua_pushinteger(lua, GL_IMAGE_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_1D_ARRAY");
//		lua_pushinteger(lua, GL_IMAGE_1D_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_2D_ARRAY");
//		lua_pushinteger(lua, GL_IMAGE_2D_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_CUBE_MAP_ARRAY");
//		lua_pushinteger(lua, GL_IMAGE_CUBE_MAP_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_2D_MULTISAMPLE");
//		lua_pushinteger(lua, GL_IMAGE_2D_MULTISAMPLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_2D_MULTISAMPLE_ARRAY");
//		lua_pushinteger(lua, GL_IMAGE_2D_MULTISAMPLE_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_1D");
//		lua_pushinteger(lua, GL_INT_IMAGE_1D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_2D");
//		lua_pushinteger(lua, GL_INT_IMAGE_2D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_3D");
//		lua_pushinteger(lua, GL_INT_IMAGE_3D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_2D_RECT");
//		lua_pushinteger(lua, GL_INT_IMAGE_2D_RECT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_CUBE");
//		lua_pushinteger(lua, GL_INT_IMAGE_CUBE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_BUFFER");
//		lua_pushinteger(lua, GL_INT_IMAGE_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_1D_ARRAY");
//		lua_pushinteger(lua, GL_INT_IMAGE_1D_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_2D_ARRAY");
//		lua_pushinteger(lua, GL_INT_IMAGE_2D_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_CUBE_MAP_ARRAY");
//		lua_pushinteger(lua, GL_INT_IMAGE_CUBE_MAP_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_2D_MULTISAMPLE");
//		lua_pushinteger(lua, GL_INT_IMAGE_2D_MULTISAMPLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INT_IMAGE_2D_MULTISAMPLE_ARRAY");
//		lua_pushinteger(lua, GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_1D");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_1D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_3D");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_3D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_RECT");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_RECT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_CUBE");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_CUBE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_BUFFER");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_1D_ARRAY");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_ARRAY");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_MULTISAMPLE");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY");
//		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_IMAGE_SAMPLES");
//		lua_pushinteger(lua, GL_MAX_IMAGE_SAMPLES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_BINDING_FORMAT");
//		lua_pushinteger(lua, GL_IMAGE_BINDING_FORMAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_FORMAT_COMPATIBILITY_TYPE");
//		lua_pushinteger(lua, GL_IMAGE_FORMAT_COMPATIBILITY_TYPE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_FORMAT_COMPATIBILITY_BY_SIZE");
//		lua_pushinteger(lua, GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IMAGE_FORMAT_COMPATIBILITY_BY_CLASS");
//		lua_pushinteger(lua, GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_VERTEX_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_CONTROL_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_EVALUATION_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_GEOMETRY_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_GEOMETRY_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAGMENT_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_FRAGMENT_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_IMAGE_UNIFORMS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_IMAGE_UNIFORMS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ALL_BARRIER_BITS");
//		lua_pushinteger(lua, GL_ALL_BARRIER_BITS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_image_size");
//		lua_pushinteger(lua, GL_ARB_shader_image_size);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shader_objects");
		lua_pushinteger(lua, GL_ARB_shader_objects);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_OBJECT_ARB");
		lua_pushinteger(lua, GL_PROGRAM_OBJECT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_OBJECT_ARB");
		lua_pushinteger(lua, GL_SHADER_OBJECT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_TYPE_ARB");
		lua_pushinteger(lua, GL_OBJECT_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_SUBTYPE_ARB");
		lua_pushinteger(lua, GL_OBJECT_SUBTYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_VEC2_ARB");
		lua_pushinteger(lua, GL_FLOAT_VEC2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_VEC3_ARB");
		lua_pushinteger(lua, GL_FLOAT_VEC3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_VEC4_ARB");
		lua_pushinteger(lua, GL_FLOAT_VEC4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_VEC2_ARB");
		lua_pushinteger(lua, GL_INT_VEC2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_VEC3_ARB");
		lua_pushinteger(lua, GL_INT_VEC3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_VEC4_ARB");
		lua_pushinteger(lua, GL_INT_VEC4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_ARB");
		lua_pushinteger(lua, GL_BOOL_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_VEC2_ARB");
		lua_pushinteger(lua, GL_BOOL_VEC2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_VEC3_ARB");
		lua_pushinteger(lua, GL_BOOL_VEC3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BOOL_VEC4_ARB");
		lua_pushinteger(lua, GL_BOOL_VEC4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT2_ARB");
		lua_pushinteger(lua, GL_FLOAT_MAT2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT3_ARB");
		lua_pushinteger(lua, GL_FLOAT_MAT3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_MAT4_ARB");
		lua_pushinteger(lua, GL_FLOAT_MAT4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_ARB");
		lua_pushinteger(lua, GL_SAMPLER_1D_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_ARB");
		lua_pushinteger(lua, GL_SAMPLER_2D_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_3D_ARB");
		lua_pushinteger(lua, GL_SAMPLER_3D_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_ARB");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_SHADOW_ARB");
		lua_pushinteger(lua, GL_SAMPLER_1D_SHADOW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_SHADOW_ARB");
		lua_pushinteger(lua, GL_SAMPLER_2D_SHADOW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_RECT_ARB");
		lua_pushinteger(lua, GL_SAMPLER_2D_RECT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_RECT_SHADOW_ARB");
		lua_pushinteger(lua, GL_SAMPLER_2D_RECT_SHADOW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_DELETE_STATUS_ARB");
		lua_pushinteger(lua, GL_OBJECT_DELETE_STATUS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_COMPILE_STATUS_ARB");
		lua_pushinteger(lua, GL_OBJECT_COMPILE_STATUS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_LINK_STATUS_ARB");
		lua_pushinteger(lua, GL_OBJECT_LINK_STATUS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_VALIDATE_STATUS_ARB");
		lua_pushinteger(lua, GL_OBJECT_VALIDATE_STATUS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_INFO_LOG_LENGTH_ARB");
		lua_pushinteger(lua, GL_OBJECT_INFO_LOG_LENGTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_ATTACHED_OBJECTS_ARB");
		lua_pushinteger(lua, GL_OBJECT_ATTACHED_OBJECTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_ACTIVE_UNIFORMS_ARB");
		lua_pushinteger(lua, GL_OBJECT_ACTIVE_UNIFORMS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB");
		lua_pushinteger(lua, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_SHADER_SOURCE_LENGTH_ARB");
		lua_pushinteger(lua, GL_OBJECT_SHADER_SOURCE_LENGTH_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_precision");
//		lua_pushinteger(lua, GL_ARB_shader_precision);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shader_stencil_export");
		lua_pushinteger(lua, GL_ARB_shader_stencil_export);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_storage_buffer_object");
//		lua_pushinteger(lua, GL_ARB_shader_storage_buffer_object);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BARRIER_BIT");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BARRIER_BIT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_SHADER_OUTPUT_RESOURCES");
//		lua_pushinteger(lua, GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BUFFER");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BUFFER_BINDING");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BUFFER_BINDING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BUFFER_START");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BUFFER_START);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BUFFER_SIZE");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BUFFER_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_GEOMETRY_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAGMENT_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMPUTE_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_COMBINED_SHADER_STORAGE_BLOCKS");
//		lua_pushinteger(lua, GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SHADER_STORAGE_BUFFER_BINDINGS");
//		lua_pushinteger(lua, GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SHADER_STORAGE_BLOCK_SIZE");
//		lua_pushinteger(lua, GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT");
//		lua_pushinteger(lua, GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shader_subroutine");
		lua_pushinteger(lua, GL_ARB_shader_subroutine);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_SUBROUTINES");
		lua_pushinteger(lua, GL_ACTIVE_SUBROUTINES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_SUBROUTINE_UNIFORMS");
		lua_pushinteger(lua, GL_ACTIVE_SUBROUTINE_UNIFORMS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SUBROUTINES");
		lua_pushinteger(lua, GL_MAX_SUBROUTINES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SUBROUTINE_UNIFORM_LOCATIONS");
		lua_pushinteger(lua, GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS");
		lua_pushinteger(lua, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_SUBROUTINE_MAX_LENGTH");
		lua_pushinteger(lua, GL_ACTIVE_SUBROUTINE_MAX_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH");
		lua_pushinteger(lua, GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_COMPATIBLE_SUBROUTINES");
		lua_pushinteger(lua, GL_NUM_COMPATIBLE_SUBROUTINES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPATIBLE_SUBROUTINES");
		lua_pushinteger(lua, GL_COMPATIBLE_SUBROUTINES);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shader_texture_image_samples");
//		lua_pushinteger(lua, GL_ARB_shader_texture_image_samples);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shader_texture_lod");
		lua_pushinteger(lua, GL_ARB_shader_texture_lod);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shading_language_100");
		lua_pushinteger(lua, GL_ARB_shading_language_100);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADING_LANGUAGE_VERSION_ARB");
		lua_pushinteger(lua, GL_SHADING_LANGUAGE_VERSION_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shading_language_420pack");
//		lua_pushinteger(lua, GL_ARB_shading_language_420pack);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shading_language_include");
		lua_pushinteger(lua, GL_ARB_shading_language_include);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_INCLUDE_ARB");
		lua_pushinteger(lua, GL_SHADER_INCLUDE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NAMED_STRING_LENGTH_ARB");
		lua_pushinteger(lua, GL_NAMED_STRING_LENGTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NAMED_STRING_TYPE_ARB");
		lua_pushinteger(lua, GL_NAMED_STRING_TYPE_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_shading_language_packing");
//		lua_pushinteger(lua, GL_ARB_shading_language_packing);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shadow");
		lua_pushinteger(lua, GL_ARB_shadow);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_MODE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_MODE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_FUNC_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_FUNC_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPARE_R_TO_TEXTURE_ARB");
		lua_pushinteger(lua, GL_COMPARE_R_TO_TEXTURE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_shadow_ambient");
		lua_pushinteger(lua, GL_ARB_shadow_ambient);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_FAIL_VALUE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_sparse_buffer");
//		lua_pushinteger(lua, GL_ARB_sparse_buffer);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SPARSE_STORAGE_BIT_ARB");
//		lua_pushinteger(lua, GL_SPARSE_STORAGE_BIT_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SPARSE_BUFFER_PAGE_SIZE_ARB");
//		lua_pushinteger(lua, GL_SPARSE_BUFFER_PAGE_SIZE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_sparse_texture");
//		lua_pushinteger(lua, GL_ARB_sparse_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_X_ARB");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_X_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_Y_ARB");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_Y_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_Z_ARB");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_Z_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SPARSE_TEXTURE_SIZE_ARB");
//		lua_pushinteger(lua, GL_MAX_SPARSE_TEXTURE_SIZE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SPARSE_3D_TEXTURE_SIZE_ARB");
//		lua_pushinteger(lua, GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB");
//		lua_pushinteger(lua, GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_SPARSE_ARB");
//		lua_pushinteger(lua, GL_TEXTURE_SPARSE_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIRTUAL_PAGE_SIZE_INDEX_ARB");
//		lua_pushinteger(lua, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NUM_VIRTUAL_PAGE_SIZES_ARB");
//		lua_pushinteger(lua, GL_NUM_VIRTUAL_PAGE_SIZES_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB");
//		lua_pushinteger(lua, GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NUM_SPARSE_LEVELS_ARB");
//		lua_pushinteger(lua, GL_NUM_SPARSE_LEVELS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_stencil_texturing");
//		lua_pushinteger(lua, GL_ARB_stencil_texturing);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEPTH_STENCIL_TEXTURE_MODE");
//		lua_pushinteger(lua, GL_DEPTH_STENCIL_TEXTURE_MODE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_sync");
		lua_pushinteger(lua, GL_ARB_sync);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_FLUSH_COMMANDS_BIT");
		lua_pushinteger(lua, GL_SYNC_FLUSH_COMMANDS_BIT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SERVER_WAIT_TIMEOUT");
		lua_pushinteger(lua, GL_MAX_SERVER_WAIT_TIMEOUT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_TYPE");
		lua_pushinteger(lua, GL_OBJECT_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_CONDITION");
		lua_pushinteger(lua, GL_SYNC_CONDITION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_STATUS");
		lua_pushinteger(lua, GL_SYNC_STATUS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_FLAGS");
		lua_pushinteger(lua, GL_SYNC_FLAGS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_FENCE");
		lua_pushinteger(lua, GL_SYNC_FENCE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SYNC_GPU_COMMANDS_COMPLETE");
		lua_pushinteger(lua, GL_SYNC_GPU_COMMANDS_COMPLETE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNALED");
		lua_pushinteger(lua, GL_UNSIGNALED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNALED");
		lua_pushinteger(lua, GL_SIGNALED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALREADY_SIGNALED");
		lua_pushinteger(lua, GL_ALREADY_SIGNALED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TIMEOUT_EXPIRED");
		lua_pushinteger(lua, GL_TIMEOUT_EXPIRED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONDITION_SATISFIED");
		lua_pushinteger(lua, GL_CONDITION_SATISFIED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WAIT_FAILED");
		lua_pushinteger(lua, GL_WAIT_FAILED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TIMEOUT_IGNORED");
		lua_pushinteger(lua, GL_TIMEOUT_IGNORED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_tessellation_shader");
		lua_pushinteger(lua, GL_ARB_tessellation_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PATCHES");
		lua_pushinteger(lua, GL_PATCHES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_CONTROL_INPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_INPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_EVALUATION_INPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PATCH_VERTICES");
		lua_pushinteger(lua, GL_PATCH_VERTICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PATCH_DEFAULT_INNER_LEVEL");
		lua_pushinteger(lua, GL_PATCH_DEFAULT_INNER_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PATCH_DEFAULT_OUTER_LEVEL");
		lua_pushinteger(lua, GL_PATCH_DEFAULT_OUTER_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_CONTROL_OUTPUT_VERTICES");
		lua_pushinteger(lua, GL_TESS_CONTROL_OUTPUT_VERTICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_GEN_MODE");
		lua_pushinteger(lua, GL_TESS_GEN_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_GEN_SPACING");
		lua_pushinteger(lua, GL_TESS_GEN_SPACING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_GEN_VERTEX_ORDER");
		lua_pushinteger(lua, GL_TESS_GEN_VERTEX_ORDER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_GEN_POINT_MODE");
		lua_pushinteger(lua, GL_TESS_GEN_POINT_MODE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ISOLINES");
		lua_pushinteger(lua, GL_ISOLINES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRACTIONAL_ODD");
		lua_pushinteger(lua, GL_FRACTIONAL_ODD);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRACTIONAL_EVEN");
		lua_pushinteger(lua, GL_FRACTIONAL_EVEN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PATCH_VERTICES");
		lua_pushinteger(lua, GL_MAX_PATCH_VERTICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_GEN_LEVEL");
		lua_pushinteger(lua, GL_MAX_TESS_GEN_LEVEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_CONTROL_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_EVALUATION_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS");
		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS");
		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_CONTROL_OUTPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_PATCH_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_PATCH_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_EVALUATION_OUTPUT_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_EVALUATION_SHADER");
		lua_pushinteger(lua, GL_TESS_EVALUATION_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_CONTROL_SHADER");
		lua_pushinteger(lua, GL_TESS_CONTROL_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_CONTROL_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TESS_EVALUATION_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_barrier");
//		lua_pushinteger(lua, GL_ARB_texture_barrier);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_border_clamp");
		lua_pushinteger(lua, GL_ARB_texture_border_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_BORDER_ARB");
		lua_pushinteger(lua, GL_CLAMP_TO_BORDER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_buffer_object");
		lua_pushinteger(lua, GL_ARB_texture_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_BUFFER_SIZE_ARB");
		lua_pushinteger(lua, GL_MAX_TEXTURE_BUFFER_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_BUFFER_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_BUFFER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_DATA_STORE_BINDING_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_FORMAT_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_FORMAT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_buffer_object_rgb32");
		lua_pushinteger(lua, GL_ARB_texture_buffer_object_rgb32);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_buffer_range");
//		lua_pushinteger(lua, GL_ARB_texture_buffer_range);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_BUFFER_OFFSET");
//		lua_pushinteger(lua, GL_TEXTURE_BUFFER_OFFSET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_BUFFER_SIZE");
//		lua_pushinteger(lua, GL_TEXTURE_BUFFER_SIZE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_BUFFER_OFFSET_ALIGNMENT");
//		lua_pushinteger(lua, GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_compression");
		lua_pushinteger(lua, GL_ARB_texture_compression);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_ALPHA_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE_ALPHA_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_INTENSITY_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_INTENSITY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGB_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSION_HINT_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSION_HINT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSED_IMAGE_SIZE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPRESSED_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COMPRESSED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_COMPRESSED_TEXTURE_FORMATS_ARB");
		lua_pushinteger(lua, GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_TEXTURE_FORMATS_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_TEXTURE_FORMATS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_compression_bptc");
		lua_pushinteger(lua, GL_ARB_texture_compression_bptc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA_BPTC_UNORM_ARB");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA_BPTC_UNORM_ARB);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB");
		// lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB");
		// lua_pushinteger(lua, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB");
		// lua_pushinteger(lua, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_compression_rgtc");
		lua_pushinteger(lua, GL_ARB_texture_compression_rgtc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RED_RGTC1");
		lua_pushinteger(lua, GL_COMPRESSED_RED_RGTC1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SIGNED_RED_RGTC1");
		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_RED_RGTC1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RG_RGTC2");
		lua_pushinteger(lua, GL_COMPRESSED_RG_RGTC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SIGNED_RG_RGTC2");
		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_RG_RGTC2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_cube_map");
		lua_pushinteger(lua, GL_ARB_texture_cube_map);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_MAP_ARB");
		lua_pushinteger(lua, GL_NORMAL_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REFLECTION_MAP_ARB");
		lua_pushinteger(lua, GL_REFLECTION_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_CUBE_MAP_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_CUBE_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_X_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_X_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Y_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Z_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP_ARB");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CUBE_MAP_TEXTURE_SIZE_ARB");
		lua_pushinteger(lua, GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_cube_map_array");
		lua_pushinteger(lua, GL_ARB_texture_cube_map_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_ARRAY_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_MAP_ARRAY_ARB");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_MAP_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_CUBE_MAP_ARRAY_ARB");
		lua_pushinteger(lua, GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_env_add");
		lua_pushinteger(lua, GL_ARB_texture_env_add);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_env_combine");
		lua_pushinteger(lua, GL_ARB_texture_env_combine);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUBTRACT_ARB");
		lua_pushinteger(lua, GL_SUBTRACT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_ARB");
		lua_pushinteger(lua, GL_COMBINE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_RGB_ARB");
		lua_pushinteger(lua, GL_COMBINE_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_ALPHA_ARB");
		lua_pushinteger(lua, GL_COMBINE_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_SCALE_ARB");
		lua_pushinteger(lua, GL_RGB_SCALE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD_SIGNED_ARB");
		lua_pushinteger(lua, GL_ADD_SIGNED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERPOLATE_ARB");
		lua_pushinteger(lua, GL_INTERPOLATE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_ARB");
		lua_pushinteger(lua, GL_CONSTANT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR_ARB");
		lua_pushinteger(lua, GL_PRIMARY_COLOR_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PREVIOUS_ARB");
		lua_pushinteger(lua, GL_PREVIOUS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_RGB_ARB");
		lua_pushinteger(lua, GL_SOURCE0_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_RGB_ARB");
		lua_pushinteger(lua, GL_SOURCE1_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_RGB_ARB");
		lua_pushinteger(lua, GL_SOURCE2_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_ALPHA_ARB");
		lua_pushinteger(lua, GL_SOURCE0_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_ALPHA_ARB");
		lua_pushinteger(lua, GL_SOURCE1_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_ALPHA_ARB");
		lua_pushinteger(lua, GL_SOURCE2_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_RGB_ARB");
		lua_pushinteger(lua, GL_OPERAND0_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_RGB_ARB");
		lua_pushinteger(lua, GL_OPERAND1_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_RGB_ARB");
		lua_pushinteger(lua, GL_OPERAND2_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_ALPHA_ARB");
		lua_pushinteger(lua, GL_OPERAND0_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_ALPHA_ARB");
		lua_pushinteger(lua, GL_OPERAND1_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_ALPHA_ARB");
		lua_pushinteger(lua, GL_OPERAND2_ALPHA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_env_crossbar");
		lua_pushinteger(lua, GL_ARB_texture_env_crossbar);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_env_dot3");
		lua_pushinteger(lua, GL_ARB_texture_env_dot3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGB_ARB");
		lua_pushinteger(lua, GL_DOT3_RGB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGBA_ARB");
		lua_pushinteger(lua, GL_DOT3_RGBA_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_float");
		lua_pushinteger(lua, GL_ARB_texture_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA32F_ARB");
		lua_pushinteger(lua, GL_RGBA32F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB32F_ARB");
		lua_pushinteger(lua, GL_RGB32F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA32F_ARB");
		lua_pushinteger(lua, GL_ALPHA32F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY32F_ARB");
		lua_pushinteger(lua, GL_INTENSITY32F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE32F_ARB");
		lua_pushinteger(lua, GL_LUMINANCE32F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA32F_ARB");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA32F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16F_ARB");
		lua_pushinteger(lua, GL_RGBA16F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16F_ARB");
		lua_pushinteger(lua, GL_RGB16F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16F_ARB");
		lua_pushinteger(lua, GL_ALPHA16F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16F_ARB");
		lua_pushinteger(lua, GL_INTENSITY16F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16F_ARB");
		lua_pushinteger(lua, GL_LUMINANCE16F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA16F_ARB");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA16F_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RED_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_RED_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GREEN_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_GREEN_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BLUE_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BLUE_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ALPHA_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_ALPHA_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LUMINANCE_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_LUMINANCE_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTENSITY_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_INTENSITY_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH_TYPE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_NORMALIZED_ARB");
		lua_pushinteger(lua, GL_UNSIGNED_NORMALIZED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_gather");
		lua_pushinteger(lua, GL_ARB_texture_gather);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB");
		lua_pushinteger(lua, GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB");
//		lua_pushinteger(lua, GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_mirror_clamp_to_edge");
//		lua_pushinteger(lua, GL_ARB_texture_mirror_clamp_to_edge);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIRROR_CLAMP_TO_EDGE");
//		lua_pushinteger(lua, GL_MIRROR_CLAMP_TO_EDGE);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_mirrored_repeat");
		lua_pushinteger(lua, GL_ARB_texture_mirrored_repeat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRRORED_REPEAT_ARB");
		lua_pushinteger(lua, GL_MIRRORED_REPEAT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_multisample");
		lua_pushinteger(lua, GL_ARB_texture_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_POSITION");
		lua_pushinteger(lua, GL_SAMPLE_POSITION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK");
		lua_pushinteger(lua, GL_SAMPLE_MASK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_VALUE");
		lua_pushinteger(lua, GL_SAMPLE_MASK_VALUE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SAMPLE_MASK_WORDS");
		lua_pushinteger(lua, GL_MAX_SAMPLE_MASK_WORDS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_MULTISAMPLE");
		lua_pushinteger(lua, GL_TEXTURE_2D_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D_MULTISAMPLE");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_MULTISAMPLE_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_2D_MULTISAMPLE");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_2D_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SAMPLES");
		lua_pushinteger(lua, GL_TEXTURE_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_FIXED_SAMPLE_LOCATIONS");
		lua_pushinteger(lua, GL_TEXTURE_FIXED_SAMPLE_LOCATIONS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_MULTISAMPLE");
		lua_pushinteger(lua, GL_SAMPLER_2D_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_MULTISAMPLE");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_MULTISAMPLE_ARRAY");
		lua_pushinteger(lua, GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_MULTISAMPLE_ARRAY");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COLOR_TEXTURE_SAMPLES");
		lua_pushinteger(lua, GL_MAX_COLOR_TEXTURE_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DEPTH_TEXTURE_SAMPLES");
		lua_pushinteger(lua, GL_MAX_DEPTH_TEXTURE_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_INTEGER_SAMPLES");
		lua_pushinteger(lua, GL_MAX_INTEGER_SAMPLES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_non_power_of_two");
		lua_pushinteger(lua, GL_ARB_texture_non_power_of_two);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_query_levels");
//		lua_pushinteger(lua, GL_ARB_texture_query_levels);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_query_lod");
		lua_pushinteger(lua, GL_ARB_texture_query_lod);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_rectangle");
		lua_pushinteger(lua, GL_ARB_texture_rectangle);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RECTANGLE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_RECTANGLE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_RECTANGLE_ARB");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_RECTANGLE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_RECTANGLE_ARB");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_RECTANGLE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_RECTANGLE_TEXTURE_SIZE_ARB");
		lua_pushinteger(lua, GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_RECT_ARB");
		lua_pushinteger(lua, GL_SAMPLER_2D_RECT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_RECT_SHADOW_ARB");
		lua_pushinteger(lua, GL_SAMPLER_2D_RECT_SHADOW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_rg");
		lua_pushinteger(lua, GL_ARB_texture_rg);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RED");
		lua_pushinteger(lua, GL_COMPRESSED_RED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RG");
		lua_pushinteger(lua, GL_COMPRESSED_RG);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG");
		lua_pushinteger(lua, GL_RG);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG_INTEGER");
		lua_pushinteger(lua, GL_RG_INTEGER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R8");
		lua_pushinteger(lua, GL_R8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R16");
		lua_pushinteger(lua, GL_R16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG8");
		lua_pushinteger(lua, GL_RG8);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16");
		lua_pushinteger(lua, GL_RG16);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R16F");
		lua_pushinteger(lua, GL_R16F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R32F");
		lua_pushinteger(lua, GL_R32F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16F");
		lua_pushinteger(lua, GL_RG16F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG32F");
		lua_pushinteger(lua, GL_RG32F);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R8I");
		lua_pushinteger(lua, GL_R8I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R8UI");
		lua_pushinteger(lua, GL_R8UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R16I");
		lua_pushinteger(lua, GL_R16I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R16UI");
		lua_pushinteger(lua, GL_R16UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R32I");
		lua_pushinteger(lua, GL_R32I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R32UI");
		lua_pushinteger(lua, GL_R32UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG8I");
		lua_pushinteger(lua, GL_RG8I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG8UI");
		lua_pushinteger(lua, GL_RG8UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16I");
		lua_pushinteger(lua, GL_RG16I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16UI");
		lua_pushinteger(lua, GL_RG16UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG32I");
		lua_pushinteger(lua, GL_RG32I);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG32UI");
		lua_pushinteger(lua, GL_RG32UI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_rgb10_a2ui");
		lua_pushinteger(lua, GL_ARB_texture_rgb10_a2ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10_A2UI");
		lua_pushinteger(lua, GL_RGB10_A2UI);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_stencil8");
//		lua_pushinteger(lua, GL_ARB_texture_stencil8);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX");
		lua_pushinteger(lua, GL_STENCIL_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX8");
		lua_pushinteger(lua, GL_STENCIL_INDEX8);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_storage");
//		lua_pushinteger(lua, GL_ARB_texture_storage);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_IMMUTABLE_FORMAT");
//		lua_pushinteger(lua, GL_TEXTURE_IMMUTABLE_FORMAT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_storage_multisample");
//		lua_pushinteger(lua, GL_ARB_texture_storage_multisample);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_texture_swizzle");
		lua_pushinteger(lua, GL_ARB_texture_swizzle);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_R");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_R);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_G");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_G);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_B");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_B);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_A");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_A);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_RGBA");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_RGBA);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_texture_view");
//		lua_pushinteger(lua, GL_ARB_texture_view);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_VIEW_MIN_LEVEL");
//		lua_pushinteger(lua, GL_TEXTURE_VIEW_MIN_LEVEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_VIEW_NUM_LEVELS");
//		lua_pushinteger(lua, GL_TEXTURE_VIEW_NUM_LEVELS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_VIEW_MIN_LAYER");
//		lua_pushinteger(lua, GL_TEXTURE_VIEW_MIN_LAYER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_VIEW_NUM_LAYERS");
//		lua_pushinteger(lua, GL_TEXTURE_VIEW_NUM_LAYERS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_IMMUTABLE_LEVELS");
//		lua_pushinteger(lua, GL_TEXTURE_IMMUTABLE_LEVELS);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_timer_query");
		lua_pushinteger(lua, GL_ARB_timer_query);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TIME_ELAPSED");
		lua_pushinteger(lua, GL_TIME_ELAPSED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TIMESTAMP");
		lua_pushinteger(lua, GL_TIMESTAMP);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_transform_feedback2");
		lua_pushinteger(lua, GL_ARB_transform_feedback2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_PAUSED");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_ACTIVE");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BINDING");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_transform_feedback3");
		lua_pushinteger(lua, GL_ARB_transform_feedback3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_BUFFERS");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_BUFFERS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_STREAMS");
		lua_pushinteger(lua, GL_MAX_VERTEX_STREAMS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_transform_feedback_instanced");
//		lua_pushinteger(lua, GL_ARB_transform_feedback_instanced);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_transform_feedback_overflow_query");
//		lua_pushinteger(lua, GL_ARB_transform_feedback_overflow_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSFORM_FEEDBACK_OVERFLOW_ARB");
//		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB");
//		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_transpose_matrix");
		lua_pushinteger(lua, GL_ARB_transpose_matrix);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_MODELVIEW_MATRIX_ARB");
		lua_pushinteger(lua, GL_TRANSPOSE_MODELVIEW_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_PROJECTION_MATRIX_ARB");
		lua_pushinteger(lua, GL_TRANSPOSE_PROJECTION_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_TEXTURE_MATRIX_ARB");
		lua_pushinteger(lua, GL_TRANSPOSE_TEXTURE_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_COLOR_MATRIX_ARB");
		lua_pushinteger(lua, GL_TRANSPOSE_COLOR_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_uniform_buffer_object");
		lua_pushinteger(lua, GL_ARB_uniform_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER_BINDING");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER_START");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER_START);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER_SIZE");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_MAX_VERTEX_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_MAX_COMBINED_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_UNIFORM_BUFFER_BINDINGS");
		lua_pushinteger(lua, GL_MAX_UNIFORM_BUFFER_BINDINGS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_UNIFORM_BLOCK_SIZE");
		lua_pushinteger(lua, GL_MAX_UNIFORM_BLOCK_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS");
		lua_pushinteger(lua, GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER_OFFSET_ALIGNMENT");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH");
		lua_pushinteger(lua, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_UNIFORM_BLOCKS");
		lua_pushinteger(lua, GL_ACTIVE_UNIFORM_BLOCKS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_TYPE");
		lua_pushinteger(lua, GL_UNIFORM_TYPE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_SIZE");
		lua_pushinteger(lua, GL_UNIFORM_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_NAME_LENGTH");
		lua_pushinteger(lua, GL_UNIFORM_NAME_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_INDEX");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_OFFSET");
		lua_pushinteger(lua, GL_UNIFORM_OFFSET);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_ARRAY_STRIDE");
		lua_pushinteger(lua, GL_UNIFORM_ARRAY_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_MATRIX_STRIDE");
		lua_pushinteger(lua, GL_UNIFORM_MATRIX_STRIDE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_IS_ROW_MAJOR");
		lua_pushinteger(lua, GL_UNIFORM_IS_ROW_MAJOR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_BINDING");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_DATA_SIZE");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_DATA_SIZE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_NAME_LENGTH");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_NAME_LENGTH);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_ACTIVE_UNIFORMS");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER");
		lua_pushinteger(lua, GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_INDEX");
		lua_pushinteger(lua, GL_INVALID_INDEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_array_bgra");
		lua_pushinteger(lua, GL_ARB_vertex_array_bgra);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA");
		lua_pushinteger(lua, GL_BGRA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_array_object");
		lua_pushinteger(lua, GL_ARB_vertex_array_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_BINDING");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_BINDING);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_attrib_64bit");
		lua_pushinteger(lua, GL_ARB_vertex_attrib_64bit);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_vertex_attrib_binding");
//		lua_pushinteger(lua, GL_ARB_vertex_attrib_binding);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_ATTRIB_BINDING");
//		lua_pushinteger(lua, GL_VERTEX_ATTRIB_BINDING);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_ATTRIB_RELATIVE_OFFSET");
//		lua_pushinteger(lua, GL_VERTEX_ATTRIB_RELATIVE_OFFSET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_BINDING_DIVISOR");
//		lua_pushinteger(lua, GL_VERTEX_BINDING_DIVISOR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_BINDING_OFFSET");
//		lua_pushinteger(lua, GL_VERTEX_BINDING_OFFSET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_BINDING_STRIDE");
//		lua_pushinteger(lua, GL_VERTEX_BINDING_STRIDE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_ATTRIB_RELATIVE_OFFSET");
//		lua_pushinteger(lua, GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_VERTEX_ATTRIB_BINDINGS");
//		lua_pushinteger(lua, GL_MAX_VERTEX_ATTRIB_BINDINGS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_BINDING_BUFFER");
//		lua_pushinteger(lua, GL_VERTEX_BINDING_BUFFER);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_blend");
		lua_pushinteger(lua, GL_ARB_vertex_blend);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW0_ARB");
		lua_pushinteger(lua, GL_MODELVIEW0_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW1_ARB");
		lua_pushinteger(lua, GL_MODELVIEW1_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_VERTEX_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_VERTEX_UNITS_ARB");
		lua_pushinteger(lua, GL_ACTIVE_VERTEX_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_SUM_UNITY_ARB");
		lua_pushinteger(lua, GL_WEIGHT_SUM_UNITY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_BLEND_ARB");
		lua_pushinteger(lua, GL_VERTEX_BLEND_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_WEIGHT_ARB");
		lua_pushinteger(lua, GL_CURRENT_WEIGHT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_TYPE_ARB");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_STRIDE_ARB");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_STRIDE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_SIZE_ARB");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_POINTER_ARB");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_POINTER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_ARB");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW2_ARB");
		lua_pushinteger(lua, GL_MODELVIEW2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW3_ARB");
		lua_pushinteger(lua, GL_MODELVIEW3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW4_ARB");
		lua_pushinteger(lua, GL_MODELVIEW4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW5_ARB");
		lua_pushinteger(lua, GL_MODELVIEW5_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW6_ARB");
		lua_pushinteger(lua, GL_MODELVIEW6_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW7_ARB");
		lua_pushinteger(lua, GL_MODELVIEW7_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW8_ARB");
		lua_pushinteger(lua, GL_MODELVIEW8_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW9_ARB");
		lua_pushinteger(lua, GL_MODELVIEW9_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW10_ARB");
		lua_pushinteger(lua, GL_MODELVIEW10_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW11_ARB");
		lua_pushinteger(lua, GL_MODELVIEW11_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW12_ARB");
		lua_pushinteger(lua, GL_MODELVIEW12_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW13_ARB");
		lua_pushinteger(lua, GL_MODELVIEW13_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW14_ARB");
		lua_pushinteger(lua, GL_MODELVIEW14_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW15_ARB");
		lua_pushinteger(lua, GL_MODELVIEW15_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW16_ARB");
		lua_pushinteger(lua, GL_MODELVIEW16_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW17_ARB");
		lua_pushinteger(lua, GL_MODELVIEW17_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW18_ARB");
		lua_pushinteger(lua, GL_MODELVIEW18_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW19_ARB");
		lua_pushinteger(lua, GL_MODELVIEW19_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW20_ARB");
		lua_pushinteger(lua, GL_MODELVIEW20_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW21_ARB");
		lua_pushinteger(lua, GL_MODELVIEW21_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW22_ARB");
		lua_pushinteger(lua, GL_MODELVIEW22_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW23_ARB");
		lua_pushinteger(lua, GL_MODELVIEW23_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW24_ARB");
		lua_pushinteger(lua, GL_MODELVIEW24_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW25_ARB");
		lua_pushinteger(lua, GL_MODELVIEW25_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW26_ARB");
		lua_pushinteger(lua, GL_MODELVIEW26_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW27_ARB");
		lua_pushinteger(lua, GL_MODELVIEW27_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW28_ARB");
		lua_pushinteger(lua, GL_MODELVIEW28_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW29_ARB");
		lua_pushinteger(lua, GL_MODELVIEW29_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW30_ARB");
		lua_pushinteger(lua, GL_MODELVIEW30_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW31_ARB");
		lua_pushinteger(lua, GL_MODELVIEW31_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_buffer_object");
		lua_pushinteger(lua, GL_ARB_vertex_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_SIZE_ARB");
		lua_pushinteger(lua, GL_BUFFER_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_USAGE_ARB");
		lua_pushinteger(lua, GL_BUFFER_USAGE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_BUFFER_ARB");
		lua_pushinteger(lua, GL_ARRAY_BUFFER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_BUFFER_ARB");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_BUFFER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_COLOR_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_INDEX_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WEIGHT_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_ONLY_ARB");
		lua_pushinteger(lua, GL_READ_ONLY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRITE_ONLY_ARB");
		lua_pushinteger(lua, GL_WRITE_ONLY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_WRITE_ARB");
		lua_pushinteger(lua, GL_READ_WRITE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_ACCESS_ARB");
		lua_pushinteger(lua, GL_BUFFER_ACCESS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_MAPPED_ARB");
		lua_pushinteger(lua, GL_BUFFER_MAPPED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_MAP_POINTER_ARB");
		lua_pushinteger(lua, GL_BUFFER_MAP_POINTER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STREAM_DRAW_ARB");
		lua_pushinteger(lua, GL_STREAM_DRAW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STREAM_READ_ARB");
		lua_pushinteger(lua, GL_STREAM_READ_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STREAM_COPY_ARB");
		lua_pushinteger(lua, GL_STREAM_COPY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_DRAW_ARB");
		lua_pushinteger(lua, GL_STATIC_DRAW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_READ_ARB");
		lua_pushinteger(lua, GL_STATIC_READ_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_COPY_ARB");
		lua_pushinteger(lua, GL_STATIC_COPY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_DRAW_ARB");
		lua_pushinteger(lua, GL_DYNAMIC_DRAW_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_READ_ARB");
		lua_pushinteger(lua, GL_DYNAMIC_READ_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_COPY_ARB");
		lua_pushinteger(lua, GL_DYNAMIC_COPY_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_program");
		lua_pushinteger(lua, GL_ARB_vertex_program);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SUM_ARB");
		lua_pushinteger(lua, GL_COLOR_SUM_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_ARB");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_ENABLED_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_SIZE_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_STRIDE_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_TYPE_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_VERTEX_ATTRIB_ARB");
		lua_pushinteger(lua, GL_CURRENT_VERTEX_ATTRIB_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_LENGTH_ARB");
		lua_pushinteger(lua, GL_PROGRAM_LENGTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_STRING_ARB");
		lua_pushinteger(lua, GL_PROGRAM_STRING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_MATRICES_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_MATRICES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_MATRIX_STACK_DEPTH_ARB");
		lua_pushinteger(lua, GL_CURRENT_MATRIX_STACK_DEPTH_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_MATRIX_ARB");
		lua_pushinteger(lua, GL_CURRENT_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_POINT_SIZE_ARB");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_TWO_SIDE_ARB");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_POINTER_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ERROR_POSITION_ARB");
		lua_pushinteger(lua, GL_PROGRAM_ERROR_POSITION_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_BINDING_ARB");
		lua_pushinteger(lua, GL_PROGRAM_BINDING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_ATTRIBS_ARB");
		lua_pushinteger(lua, GL_MAX_VERTEX_ATTRIBS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ERROR_STRING_ARB");
		lua_pushinteger(lua, GL_PROGRAM_ERROR_STRING_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_FORMAT_ASCII_ARB");
		lua_pushinteger(lua, GL_PROGRAM_FORMAT_ASCII_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_FORMAT_ARB");
		lua_pushinteger(lua, GL_PROGRAM_FORMAT_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_TEMPORARIES_ARB");
		lua_pushinteger(lua, GL_PROGRAM_TEMPORARIES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEMPORARIES_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEMPORARIES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_TEMPORARIES_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_TEMPORARIES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_TEMPORARIES_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_PARAMETERS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_PARAMETERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_PARAMETERS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_PARAMETERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_PARAMETERS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_PARAMETERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_PARAMETERS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ATTRIBS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_ATTRIBS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_ATTRIBS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_ATTRIBS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_ATTRIBS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_ATTRIBS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_ATTRIBS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ADDRESS_REGISTERS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_ADDRESS_REGISTERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_ADDRESS_REGISTERS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_LOCAL_PARAMETERS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_ENV_PARAMETERS_ARB");
		lua_pushinteger(lua, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_UNDER_NATIVE_LIMITS_ARB");
		lua_pushinteger(lua, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_CURRENT_MATRIX_ARB");
		lua_pushinteger(lua, GL_TRANSPOSE_CURRENT_MATRIX_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX0_ARB");
		lua_pushinteger(lua, GL_MATRIX0_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX1_ARB");
		lua_pushinteger(lua, GL_MATRIX1_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX2_ARB");
		lua_pushinteger(lua, GL_MATRIX2_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX3_ARB");
		lua_pushinteger(lua, GL_MATRIX3_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX4_ARB");
		lua_pushinteger(lua, GL_MATRIX4_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX5_ARB");
		lua_pushinteger(lua, GL_MATRIX5_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX6_ARB");
		lua_pushinteger(lua, GL_MATRIX6_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX7_ARB");
		lua_pushinteger(lua, GL_MATRIX7_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX8_ARB");
		lua_pushinteger(lua, GL_MATRIX8_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX9_ARB");
		lua_pushinteger(lua, GL_MATRIX9_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX10_ARB");
		lua_pushinteger(lua, GL_MATRIX10_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX11_ARB");
		lua_pushinteger(lua, GL_MATRIX11_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX12_ARB");
		lua_pushinteger(lua, GL_MATRIX12_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX13_ARB");
		lua_pushinteger(lua, GL_MATRIX13_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX14_ARB");
		lua_pushinteger(lua, GL_MATRIX14_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX15_ARB");
		lua_pushinteger(lua, GL_MATRIX15_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX16_ARB");
		lua_pushinteger(lua, GL_MATRIX16_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX17_ARB");
		lua_pushinteger(lua, GL_MATRIX17_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX18_ARB");
		lua_pushinteger(lua, GL_MATRIX18_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX19_ARB");
		lua_pushinteger(lua, GL_MATRIX19_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX20_ARB");
		lua_pushinteger(lua, GL_MATRIX20_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX21_ARB");
		lua_pushinteger(lua, GL_MATRIX21_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX22_ARB");
		lua_pushinteger(lua, GL_MATRIX22_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX23_ARB");
		lua_pushinteger(lua, GL_MATRIX23_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX24_ARB");
		lua_pushinteger(lua, GL_MATRIX24_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX25_ARB");
		lua_pushinteger(lua, GL_MATRIX25_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX26_ARB");
		lua_pushinteger(lua, GL_MATRIX26_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX27_ARB");
		lua_pushinteger(lua, GL_MATRIX27_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX28_ARB");
		lua_pushinteger(lua, GL_MATRIX28_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX29_ARB");
		lua_pushinteger(lua, GL_MATRIX29_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX30_ARB");
		lua_pushinteger(lua, GL_MATRIX30_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX31_ARB");
		lua_pushinteger(lua, GL_MATRIX31_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_shader");
		lua_pushinteger(lua, GL_ARB_vertex_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_ARB");
		lua_pushinteger(lua, GL_VERTEX_SHADER_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_UNIFORM_COMPONENTS_ARB");
		lua_pushinteger(lua, GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VARYING_FLOATS_ARB");
		lua_pushinteger(lua, GL_MAX_VARYING_FLOATS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB");
		lua_pushinteger(lua, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_ACTIVE_ATTRIBUTES_ARB");
		lua_pushinteger(lua, GL_OBJECT_ACTIVE_ATTRIBUTES_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB");
		lua_pushinteger(lua, GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARB_vertex_type_10f_11f_11f_rev");
//		lua_pushinteger(lua, GL_ARB_vertex_type_10f_11f_11f_rev);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_10F_11F_11F_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_10F_11F_11F_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_vertex_type_2_10_10_10_rev");
		lua_pushinteger(lua, GL_ARB_vertex_type_2_10_10_10_rev);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_2_10_10_10_REV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_2_10_10_10_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_2_10_10_10_REV");
		lua_pushinteger(lua, GL_INT_2_10_10_10_REV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_viewport_array");
		lua_pushinteger(lua, GL_ARB_viewport_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_RANGE");
		lua_pushinteger(lua, GL_DEPTH_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT");
		lua_pushinteger(lua, GL_VIEWPORT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_BOX");
		lua_pushinteger(lua, GL_SCISSOR_BOX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCISSOR_TEST");
		lua_pushinteger(lua, GL_SCISSOR_TEST);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VIEWPORTS");
		lua_pushinteger(lua, GL_MAX_VIEWPORTS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT_SUBPIXEL_BITS");
		lua_pushinteger(lua, GL_VIEWPORT_SUBPIXEL_BITS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT_BOUNDS_RANGE");
		lua_pushinteger(lua, GL_VIEWPORT_BOUNDS_RANGE);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LAYER_PROVOKING_VERTEX");
		lua_pushinteger(lua, GL_LAYER_PROVOKING_VERTEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIEWPORT_INDEX_PROVOKING_VERTEX");
		lua_pushinteger(lua, GL_VIEWPORT_INDEX_PROVOKING_VERTEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNDEFINED_VERTEX");
		lua_pushinteger(lua, GL_UNDEFINED_VERTEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIRST_VERTEX_CONVENTION");
		lua_pushinteger(lua, GL_FIRST_VERTEX_CONVENTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LAST_VERTEX_CONVENTION");
		lua_pushinteger(lua, GL_LAST_VERTEX_CONVENTION);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROVOKING_VERTEX");
		lua_pushinteger(lua, GL_PROVOKING_VERTEX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARB_window_pos");
		lua_pushinteger(lua, GL_ARB_window_pos);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATIX_point_sprites");
//		lua_pushinteger(lua, GL_ATIX_point_sprites);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_POINT_MODE_ATIX");
//		lua_pushinteger(lua, GL_TEXTURE_POINT_MODE_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_POINT_ONE_COORD_ATIX");
//		lua_pushinteger(lua, GL_TEXTURE_POINT_ONE_COORD_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_POINT_SPRITE_ATIX");
//		lua_pushinteger(lua, GL_TEXTURE_POINT_SPRITE_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "POINT_SPRITE_CULL_MODE_ATIX");
//		lua_pushinteger(lua, GL_POINT_SPRITE_CULL_MODE_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "POINT_SPRITE_CULL_CENTER_ATIX");
//		lua_pushinteger(lua, GL_POINT_SPRITE_CULL_CENTER_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "POINT_SPRITE_CULL_CLIP_ATIX");
//		lua_pushinteger(lua, GL_POINT_SPRITE_CULL_CLIP_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATIX_texture_env_combine3");
//		lua_pushinteger(lua, GL_ATIX_texture_env_combine3);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MODULATE_ADD_ATIX");
//		lua_pushinteger(lua, GL_MODULATE_ADD_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MODULATE_SIGNED_ADD_ATIX");
//		lua_pushinteger(lua, GL_MODULATE_SIGNED_ADD_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MODULATE_SUBTRACT_ATIX");
//		lua_pushinteger(lua, GL_MODULATE_SUBTRACT_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATIX_texture_env_route");
//		lua_pushinteger(lua, GL_ATIX_texture_env_route);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SECONDARY_COLOR_ATIX");
//		lua_pushinteger(lua, GL_SECONDARY_COLOR_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_OUTPUT_RGB_ATIX");
//		lua_pushinteger(lua, GL_TEXTURE_OUTPUT_RGB_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_OUTPUT_ALPHA_ATIX");
//		lua_pushinteger(lua, GL_TEXTURE_OUTPUT_ALPHA_ATIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATIX_vertex_shader_output_point_size");
//		lua_pushinteger(lua, GL_ATIX_vertex_shader_output_point_size);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "OUTPUT_POINT_SIZE_ATIX");
//		lua_pushinteger(lua, GL_OUTPUT_POINT_SIZE_ATIX);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_draw_buffers");
		lua_pushinteger(lua, GL_ATI_draw_buffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_DRAW_BUFFERS_ATI");
		lua_pushinteger(lua, GL_MAX_DRAW_BUFFERS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER0_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER0_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER1_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER1_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER2_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER2_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER3_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER3_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER4_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER4_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER5_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER5_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER6_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER6_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER7_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER7_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER8_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER8_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER9_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER9_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER10_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER10_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER11_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER11_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER12_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER12_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER13_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER13_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER14_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER14_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_BUFFER15_ATI");
		lua_pushinteger(lua, GL_DRAW_BUFFER15_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_element_array");
		lua_pushinteger(lua, GL_ATI_element_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_ATI");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_TYPE_ATI");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_TYPE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_POINTER_ATI");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_POINTER_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_envmap_bumpmap");
		lua_pushinteger(lua, GL_ATI_envmap_bumpmap);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUMP_ROT_MATRIX_ATI");
		lua_pushinteger(lua, GL_BUMP_ROT_MATRIX_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUMP_ROT_MATRIX_SIZE_ATI");
		lua_pushinteger(lua, GL_BUMP_ROT_MATRIX_SIZE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUMP_NUM_TEX_UNITS_ATI");
		lua_pushinteger(lua, GL_BUMP_NUM_TEX_UNITS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUMP_TEX_UNITS_ATI");
		lua_pushinteger(lua, GL_BUMP_TEX_UNITS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DUDV_ATI");
		lua_pushinteger(lua, GL_DUDV_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DU8DV8_ATI");
		lua_pushinteger(lua, GL_DU8DV8_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUMP_ENVMAP_ATI");
		lua_pushinteger(lua, GL_BUMP_ENVMAP_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUMP_TARGET_ATI");
		lua_pushinteger(lua, GL_BUMP_TARGET_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_fragment_shader");
		lua_pushinteger(lua, GL_ATI_fragment_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2X_BIT_ATI");
		lua_pushinteger(lua, GL_2X_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_BIT_ATI");
		lua_pushinteger(lua, GL_RED_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4X_BIT_ATI");
		lua_pushinteger(lua, GL_4X_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMP_BIT_ATI");
		lua_pushinteger(lua, GL_COMP_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_BIT_ATI");
		lua_pushinteger(lua, GL_GREEN_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "8X_BIT_ATI");
		lua_pushinteger(lua, GL_8X_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_BIT_ATI");
		lua_pushinteger(lua, GL_BLUE_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEGATE_BIT_ATI");
		lua_pushinteger(lua, GL_NEGATE_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BIAS_BIT_ATI");
		lua_pushinteger(lua, GL_BIAS_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_BIT_ATI");
		lua_pushinteger(lua, GL_HALF_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUARTER_BIT_ATI");
		lua_pushinteger(lua, GL_QUARTER_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EIGHTH_BIT_ATI");
		lua_pushinteger(lua, GL_EIGHTH_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SATURATE_BIT_ATI");
		lua_pushinteger(lua, GL_SATURATE_BIT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_SHADER_ATI");
		lua_pushinteger(lua, GL_FRAGMENT_SHADER_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REG_0_ATI");
		lua_pushinteger(lua, GL_REG_0_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REG_1_ATI");
		lua_pushinteger(lua, GL_REG_1_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REG_2_ATI");
		lua_pushinteger(lua, GL_REG_2_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REG_3_ATI");
		lua_pushinteger(lua, GL_REG_3_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REG_4_ATI");
		lua_pushinteger(lua, GL_REG_4_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REG_5_ATI");
		lua_pushinteger(lua, GL_REG_5_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_0_ATI");
		lua_pushinteger(lua, GL_CON_0_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_1_ATI");
		lua_pushinteger(lua, GL_CON_1_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_2_ATI");
		lua_pushinteger(lua, GL_CON_2_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_3_ATI");
		lua_pushinteger(lua, GL_CON_3_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_4_ATI");
		lua_pushinteger(lua, GL_CON_4_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_5_ATI");
		lua_pushinteger(lua, GL_CON_5_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_6_ATI");
		lua_pushinteger(lua, GL_CON_6_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CON_7_ATI");
		lua_pushinteger(lua, GL_CON_7_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MOV_ATI");
		lua_pushinteger(lua, GL_MOV_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD_ATI");
		lua_pushinteger(lua, GL_ADD_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MUL_ATI");
		lua_pushinteger(lua, GL_MUL_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUB_ATI");
		lua_pushinteger(lua, GL_SUB_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_ATI");
		lua_pushinteger(lua, GL_DOT3_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT4_ATI");
		lua_pushinteger(lua, GL_DOT4_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAD_ATI");
		lua_pushinteger(lua, GL_MAD_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LERP_ATI");
		lua_pushinteger(lua, GL_LERP_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CND_ATI");
		lua_pushinteger(lua, GL_CND_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CND0_ATI");
		lua_pushinteger(lua, GL_CND0_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT2_ADD_ATI");
		lua_pushinteger(lua, GL_DOT2_ADD_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_INTERPOLATOR_ATI");
		lua_pushinteger(lua, GL_SECONDARY_INTERPOLATOR_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_FRAGMENT_REGISTERS_ATI");
		lua_pushinteger(lua, GL_NUM_FRAGMENT_REGISTERS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_FRAGMENT_CONSTANTS_ATI");
		lua_pushinteger(lua, GL_NUM_FRAGMENT_CONSTANTS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_PASSES_ATI");
		lua_pushinteger(lua, GL_NUM_PASSES_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_INSTRUCTIONS_PER_PASS_ATI");
		lua_pushinteger(lua, GL_NUM_INSTRUCTIONS_PER_PASS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_INSTRUCTIONS_TOTAL_ATI");
		lua_pushinteger(lua, GL_NUM_INSTRUCTIONS_TOTAL_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI");
		lua_pushinteger(lua, GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_LOOPBACK_COMPONENTS_ATI");
		lua_pushinteger(lua, GL_NUM_LOOPBACK_COMPONENTS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ALPHA_PAIRING_ATI");
		lua_pushinteger(lua, GL_COLOR_ALPHA_PAIRING_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SWIZZLE_STR_ATI");
		lua_pushinteger(lua, GL_SWIZZLE_STR_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SWIZZLE_STQ_ATI");
		lua_pushinteger(lua, GL_SWIZZLE_STQ_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SWIZZLE_STR_DR_ATI");
		lua_pushinteger(lua, GL_SWIZZLE_STR_DR_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SWIZZLE_STQ_DQ_ATI");
		lua_pushinteger(lua, GL_SWIZZLE_STQ_DQ_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SWIZZLE_STRQ_ATI");
		lua_pushinteger(lua, GL_SWIZZLE_STRQ_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SWIZZLE_STRQ_DQ_ATI");
		lua_pushinteger(lua, GL_SWIZZLE_STRQ_DQ_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_map_object_buffer");
		lua_pushinteger(lua, GL_ATI_map_object_buffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_meminfo");
		lua_pushinteger(lua, GL_ATI_meminfo);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VBO_FREE_MEMORY_ATI");
		lua_pushinteger(lua, GL_VBO_FREE_MEMORY_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_FREE_MEMORY_ATI");
		lua_pushinteger(lua, GL_TEXTURE_FREE_MEMORY_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_FREE_MEMORY_ATI");
		lua_pushinteger(lua, GL_RENDERBUFFER_FREE_MEMORY_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_pn_triangles");
		lua_pushinteger(lua, GL_ATI_pn_triangles);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI");
		lua_pushinteger(lua, GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_POINT_MODE_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_POINT_MODE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_NORMAL_MODE_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_NORMAL_MODE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_TESSELATION_LEVEL_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_POINT_MODE_LINEAR_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_POINT_MODE_CUBIC_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI");
		lua_pushinteger(lua, GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_separate_stencil");
		lua_pushinteger(lua, GL_ATI_separate_stencil);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_FUNC_ATI");
		lua_pushinteger(lua, GL_STENCIL_BACK_FUNC_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_FAIL_ATI");
		lua_pushinteger(lua, GL_STENCIL_BACK_FAIL_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_PASS_DEPTH_FAIL_ATI");
		lua_pushinteger(lua, GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_BACK_PASS_DEPTH_PASS_ATI");
		lua_pushinteger(lua, GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATI_shader_texture_lod");
//		lua_pushinteger(lua, GL_ATI_shader_texture_lod);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_text_fragment_shader");
		lua_pushinteger(lua, GL_ATI_text_fragment_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXT_FRAGMENT_SHADER_ATI");
		lua_pushinteger(lua, GL_TEXT_FRAGMENT_SHADER_ATI);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "ATI_texture_compression_3dc");
//		lua_pushinteger(lua, GL_ATI_texture_compression_3dc);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_LUMINANCE_ALPHA_3DC_ATI");
//		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_texture_env_combine3");
		lua_pushinteger(lua, GL_ATI_texture_env_combine3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODULATE_ADD_ATI");
		lua_pushinteger(lua, GL_MODULATE_ADD_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODULATE_SIGNED_ADD_ATI");
		lua_pushinteger(lua, GL_MODULATE_SIGNED_ADD_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODULATE_SUBTRACT_ATI");
		lua_pushinteger(lua, GL_MODULATE_SUBTRACT_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_texture_float");
		lua_pushinteger(lua, GL_ATI_texture_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_FLOAT32_ATI");
		lua_pushinteger(lua, GL_RGBA_FLOAT32_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_FLOAT32_ATI");
		lua_pushinteger(lua, GL_RGB_FLOAT32_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_FLOAT32_ATI");
		lua_pushinteger(lua, GL_ALPHA_FLOAT32_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY_FLOAT32_ATI");
		lua_pushinteger(lua, GL_INTENSITY_FLOAT32_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_FLOAT32_ATI");
		lua_pushinteger(lua, GL_LUMINANCE_FLOAT32_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA_FLOAT32_ATI");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_FLOAT32_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_FLOAT16_ATI");
		lua_pushinteger(lua, GL_RGBA_FLOAT16_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_FLOAT16_ATI");
		lua_pushinteger(lua, GL_RGB_FLOAT16_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_FLOAT16_ATI");
		lua_pushinteger(lua, GL_ALPHA_FLOAT16_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY_FLOAT16_ATI");
		lua_pushinteger(lua, GL_INTENSITY_FLOAT16_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_FLOAT16_ATI");
		lua_pushinteger(lua, GL_LUMINANCE_FLOAT16_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA_FLOAT16_ATI");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_FLOAT16_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_texture_mirror_once");
		lua_pushinteger(lua, GL_ATI_texture_mirror_once);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRROR_CLAMP_ATI");
		lua_pushinteger(lua, GL_MIRROR_CLAMP_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRROR_CLAMP_TO_EDGE_ATI");
		lua_pushinteger(lua, GL_MIRROR_CLAMP_TO_EDGE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_vertex_array_object");
		lua_pushinteger(lua, GL_ATI_vertex_array_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STATIC_ATI");
		lua_pushinteger(lua, GL_STATIC_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DYNAMIC_ATI");
		lua_pushinteger(lua, GL_DYNAMIC_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRESERVE_ATI");
		lua_pushinteger(lua, GL_PRESERVE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DISCARD_ATI");
		lua_pushinteger(lua, GL_DISCARD_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_BUFFER_SIZE_ATI");
		lua_pushinteger(lua, GL_OBJECT_BUFFER_SIZE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_BUFFER_USAGE_ATI");
		lua_pushinteger(lua, GL_OBJECT_BUFFER_USAGE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_OBJECT_BUFFER_ATI");
		lua_pushinteger(lua, GL_ARRAY_OBJECT_BUFFER_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_OBJECT_OFFSET_ATI");
		lua_pushinteger(lua, GL_ARRAY_OBJECT_OFFSET_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_vertex_attrib_array_object");
		lua_pushinteger(lua, GL_ATI_vertex_attrib_array_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATI_vertex_streams");
		lua_pushinteger(lua, GL_ATI_vertex_streams);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_STREAMS_ATI");
		lua_pushinteger(lua, GL_MAX_VERTEX_STREAMS_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SOURCE_ATI");
		lua_pushinteger(lua, GL_VERTEX_SOURCE_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM0_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM0_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM1_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM1_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM2_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM2_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM3_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM3_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM4_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM4_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM5_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM5_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM6_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM6_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STREAM7_ATI");
		lua_pushinteger(lua, GL_VERTEX_STREAM7_ATI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_422_pixels");
		lua_pushinteger(lua, GL_EXT_422_pixels);
		lua_settable(lua, -3);
		lua_pushstring(lua, "422_EXT");
		lua_pushinteger(lua, GL_422_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "422_REV_EXT");
		lua_pushinteger(lua, GL_422_REV_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "422_AVERAGE_EXT");
		lua_pushinteger(lua, GL_422_AVERAGE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "422_REV_AVERAGE_EXT");
		lua_pushinteger(lua, GL_422_REV_AVERAGE_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_Cg_shader");
//		lua_pushinteger(lua, GL_EXT_Cg_shader);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CG_VERTEX_SHADER_EXT");
//		lua_pushinteger(lua, GL_CG_VERTEX_SHADER_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CG_FRAGMENT_SHADER_EXT");
//		lua_pushinteger(lua, GL_CG_FRAGMENT_SHADER_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_abgr");
		lua_pushinteger(lua, GL_EXT_abgr);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ABGR_EXT");
		lua_pushinteger(lua, GL_ABGR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_bgra");
		lua_pushinteger(lua, GL_EXT_bgra);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGR_EXT");
		lua_pushinteger(lua, GL_BGR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA_EXT");
		lua_pushinteger(lua, GL_BGRA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_bindable_uniform");
		lua_pushinteger(lua, GL_EXT_bindable_uniform);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_BINDABLE_UNIFORMS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_BINDABLE_UNIFORM_SIZE_EXT");
		lua_pushinteger(lua, GL_MAX_BINDABLE_UNIFORM_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER_EXT");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_UNIFORM_BUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_blend_color");
		lua_pushinteger(lua, GL_EXT_blend_color);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_COLOR_EXT");
		lua_pushinteger(lua, GL_CONSTANT_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_CONSTANT_COLOR_EXT");
		lua_pushinteger(lua, GL_ONE_MINUS_CONSTANT_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_ALPHA_EXT");
		lua_pushinteger(lua, GL_CONSTANT_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_MINUS_CONSTANT_ALPHA_EXT");
		lua_pushinteger(lua, GL_ONE_MINUS_CONSTANT_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_COLOR_EXT");
		lua_pushinteger(lua, GL_BLEND_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_blend_equation_separate");
		lua_pushinteger(lua, GL_EXT_blend_equation_separate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION_RGB_EXT");
		lua_pushinteger(lua, GL_BLEND_EQUATION_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION_ALPHA_EXT");
		lua_pushinteger(lua, GL_BLEND_EQUATION_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_blend_func_separate");
		lua_pushinteger(lua, GL_EXT_blend_func_separate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_DST_RGB_EXT");
		lua_pushinteger(lua, GL_BLEND_DST_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_SRC_RGB_EXT");
		lua_pushinteger(lua, GL_BLEND_SRC_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_DST_ALPHA_EXT");
		lua_pushinteger(lua, GL_BLEND_DST_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_SRC_ALPHA_EXT");
		lua_pushinteger(lua, GL_BLEND_SRC_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_blend_logic_op");
		lua_pushinteger(lua, GL_EXT_blend_logic_op);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_blend_minmax");
		lua_pushinteger(lua, GL_EXT_blend_minmax);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_ADD_EXT");
		lua_pushinteger(lua, GL_FUNC_ADD_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_EXT");
		lua_pushinteger(lua, GL_MIN_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_EXT");
		lua_pushinteger(lua, GL_MAX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLEND_EQUATION_EXT");
		lua_pushinteger(lua, GL_BLEND_EQUATION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_blend_subtract");
		lua_pushinteger(lua, GL_EXT_blend_subtract);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_SUBTRACT_EXT");
		lua_pushinteger(lua, GL_FUNC_SUBTRACT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FUNC_REVERSE_SUBTRACT_EXT");
		lua_pushinteger(lua, GL_FUNC_REVERSE_SUBTRACT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_clip_volume_hint");
		lua_pushinteger(lua, GL_EXT_clip_volume_hint);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_VOLUME_CLIPPING_HINT_EXT");
		lua_pushinteger(lua, GL_CLIP_VOLUME_CLIPPING_HINT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_cmyka");
		lua_pushinteger(lua, GL_EXT_cmyka);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CMYK_EXT");
		lua_pushinteger(lua, GL_CMYK_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CMYKA_EXT");
		lua_pushinteger(lua, GL_CMYKA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_CMYK_HINT_EXT");
		lua_pushinteger(lua, GL_PACK_CMYK_HINT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_CMYK_HINT_EXT");
		lua_pushinteger(lua, GL_UNPACK_CMYK_HINT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_color_subtable");
		lua_pushinteger(lua, GL_EXT_color_subtable);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_compiled_vertex_array");
		lua_pushinteger(lua, GL_EXT_compiled_vertex_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_ELEMENT_LOCK_FIRST_EXT");
		lua_pushinteger(lua, GL_ARRAY_ELEMENT_LOCK_FIRST_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ARRAY_ELEMENT_LOCK_COUNT_EXT");
		lua_pushinteger(lua, GL_ARRAY_ELEMENT_LOCK_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_convolution");
		lua_pushinteger(lua, GL_EXT_convolution);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_1D_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_2D_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARABLE_2D_EXT");
		lua_pushinteger(lua, GL_SEPARABLE_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_BORDER_MODE_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_BORDER_MODE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FILTER_SCALE_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_FILTER_SCALE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FILTER_BIAS_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_FILTER_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REDUCE_EXT");
		lua_pushinteger(lua, GL_REDUCE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_FORMAT_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_FORMAT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_WIDTH_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_WIDTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_HEIGHT_EXT");
		lua_pushinteger(lua, GL_CONVOLUTION_HEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CONVOLUTION_WIDTH_EXT");
		lua_pushinteger(lua, GL_MAX_CONVOLUTION_WIDTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CONVOLUTION_HEIGHT_EXT");
		lua_pushinteger(lua, GL_MAX_CONVOLUTION_HEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_RED_SCALE_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_RED_SCALE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_GREEN_SCALE_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_GREEN_SCALE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_BLUE_SCALE_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_BLUE_SCALE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_ALPHA_SCALE_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_ALPHA_SCALE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_RED_BIAS_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_RED_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_GREEN_BIAS_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_GREEN_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_BLUE_BIAS_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_BLUE_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_ALPHA_BIAS_EXT");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_ALPHA_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_coordinate_frame");
		lua_pushinteger(lua, GL_EXT_coordinate_frame);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TANGENT_ARRAY_EXT");
		lua_pushinteger(lua, GL_TANGENT_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BINORMAL_ARRAY_EXT");
		lua_pushinteger(lua, GL_BINORMAL_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_TANGENT_EXT");
		lua_pushinteger(lua, GL_CURRENT_TANGENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_BINORMAL_EXT");
		lua_pushinteger(lua, GL_CURRENT_BINORMAL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TANGENT_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_TANGENT_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TANGENT_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_TANGENT_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BINORMAL_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_BINORMAL_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BINORMAL_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_BINORMAL_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TANGENT_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_TANGENT_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BINORMAL_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_BINORMAL_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_TANGENT_EXT");
		lua_pushinteger(lua, GL_MAP1_TANGENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_TANGENT_EXT");
		lua_pushinteger(lua, GL_MAP2_TANGENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_BINORMAL_EXT");
		lua_pushinteger(lua, GL_MAP1_BINORMAL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_BINORMAL_EXT");
		lua_pushinteger(lua, GL_MAP2_BINORMAL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_copy_texture");
		lua_pushinteger(lua, GL_EXT_copy_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_cull_vertex");
		lua_pushinteger(lua, GL_EXT_cull_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_VERTEX_EXT");
		lua_pushinteger(lua, GL_CULL_VERTEX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_VERTEX_EYE_POSITION_EXT");
		lua_pushinteger(lua, GL_CULL_VERTEX_EYE_POSITION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_VERTEX_OBJECT_POSITION_EXT");
		lua_pushinteger(lua, GL_CULL_VERTEX_OBJECT_POSITION_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_debug_label");
//		lua_pushinteger(lua, GL_EXT_debug_label);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM_PIPELINE_OBJECT_EXT");
//		lua_pushinteger(lua, GL_PROGRAM_PIPELINE_OBJECT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM_OBJECT_EXT");
//		lua_pushinteger(lua, GL_PROGRAM_OBJECT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER_OBJECT_EXT");
//		lua_pushinteger(lua, GL_SHADER_OBJECT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER_OBJECT_EXT");
//		lua_pushinteger(lua, GL_BUFFER_OBJECT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_OBJECT_EXT");
//		lua_pushinteger(lua, GL_QUERY_OBJECT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_ARRAY_OBJECT_EXT");
//		lua_pushinteger(lua, GL_VERTEX_ARRAY_OBJECT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_debug_marker");
//		lua_pushinteger(lua, GL_EXT_debug_marker);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_depth_bounds_test");
		lua_pushinteger(lua, GL_EXT_depth_bounds_test);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BOUNDS_TEST_EXT");
		lua_pushinteger(lua, GL_DEPTH_BOUNDS_TEST_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BOUNDS_EXT");
		lua_pushinteger(lua, GL_DEPTH_BOUNDS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_direct_state_access");
		lua_pushinteger(lua, GL_EXT_direct_state_access);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_MATRIX_EXT");
		lua_pushinteger(lua, GL_PROGRAM_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_PROGRAM_MATRIX_EXT");
		lua_pushinteger(lua, GL_TRANSPOSE_PROGRAM_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_MATRIX_STACK_DEPTH_EXT");
		lua_pushinteger(lua, GL_PROGRAM_MATRIX_STACK_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_draw_buffers2");
		lua_pushinteger(lua, GL_EXT_draw_buffers2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_draw_instanced");
		lua_pushinteger(lua, GL_EXT_draw_instanced);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_draw_range_elements");
		lua_pushinteger(lua, GL_EXT_draw_range_elements);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ELEMENTS_VERTICES_EXT");
		lua_pushinteger(lua, GL_MAX_ELEMENTS_VERTICES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ELEMENTS_INDICES_EXT");
		lua_pushinteger(lua, GL_MAX_ELEMENTS_INDICES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_fog_coord");
		lua_pushinteger(lua, GL_EXT_fog_coord);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_SOURCE_EXT");
		lua_pushinteger(lua, GL_FOG_COORDINATE_SOURCE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_EXT");
		lua_pushinteger(lua, GL_FOG_COORDINATE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_DEPTH_EXT");
		lua_pushinteger(lua, GL_FRAGMENT_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_FOG_COORDINATE_EXT");
		lua_pushinteger(lua, GL_CURRENT_FOG_COORDINATE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_EXT");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_fragment_lighting");
//		lua_pushinteger(lua, GL_EXT_fragment_lighting);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHTING_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHTING_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_COLOR_MATERIAL_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_COLOR_MATERIAL_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_COLOR_MATERIAL_FACE_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_COLOR_MATERIAL_FACE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_COLOR_MATERIAL_PARAMETER_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_FRAGMENT_LIGHTS_EXT");
//		lua_pushinteger(lua, GL_MAX_FRAGMENT_LIGHTS_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_ACTIVE_LIGHTS_EXT");
//		lua_pushinteger(lua, GL_MAX_ACTIVE_LIGHTS_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CURRENT_RASTER_NORMAL_EXT");
//		lua_pushinteger(lua, GL_CURRENT_RASTER_NORMAL_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LIGHT_ENV_MODE_EXT");
//		lua_pushinteger(lua, GL_LIGHT_ENV_MODE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHT_MODEL_TWO_SIDE_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHT_MODEL_AMBIENT_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHT_MODEL_AMBIENT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHT0_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHT0_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_LIGHT7_EXT");
//		lua_pushinteger(lua, GL_FRAGMENT_LIGHT7_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_framebuffer_blit");
		lua_pushinteger(lua, GL_EXT_framebuffer_blit);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_FRAMEBUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_DRAW_FRAMEBUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_FRAMEBUFFER_EXT");
		lua_pushinteger(lua, GL_READ_FRAMEBUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_FRAMEBUFFER_EXT");
		lua_pushinteger(lua, GL_DRAW_FRAMEBUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_FRAMEBUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_READ_FRAMEBUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_framebuffer_multisample");
		lua_pushinteger(lua, GL_EXT_framebuffer_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_SAMPLES_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_SAMPLES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SAMPLES_EXT");
		lua_pushinteger(lua, GL_MAX_SAMPLES_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_framebuffer_multisample_blit_scaled");
//		lua_pushinteger(lua, GL_EXT_framebuffer_multisample_blit_scaled);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SCALED_RESOLVE_FASTEST_EXT");
//		lua_pushinteger(lua, GL_SCALED_RESOLVE_FASTEST_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SCALED_RESOLVE_NICEST_EXT");
//		lua_pushinteger(lua, GL_SCALED_RESOLVE_NICEST_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_framebuffer_object");
		lua_pushinteger(lua, GL_EXT_framebuffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVALID_FRAMEBUFFER_OPERATION_EXT");
		lua_pushinteger(lua, GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_RENDERBUFFER_SIZE_EXT");
		lua_pushinteger(lua, GL_MAX_RENDERBUFFER_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_COMPLETE_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_COMPLETE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_UNSUPPORTED_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_UNSUPPORTED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COLOR_ATTACHMENTS_EXT");
		lua_pushinteger(lua, GL_MAX_COLOR_ATTACHMENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT0_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT0_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT1_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT2_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT3_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT4_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT5_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT5_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT6_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT6_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT7_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT7_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT8_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT9_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT9_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT10_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT10_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT11_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT11_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT12_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT13_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT13_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT14_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT14_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ATTACHMENT15_EXT");
		lua_pushinteger(lua, GL_COLOR_ATTACHMENT15_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_ATTACHMENT_EXT");
		lua_pushinteger(lua, GL_DEPTH_ATTACHMENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_ATTACHMENT_EXT");
		lua_pushinteger(lua, GL_STENCIL_ATTACHMENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_WIDTH_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_WIDTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_HEIGHT_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_HEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_INTERNAL_FORMAT_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_INTERNAL_FORMAT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX1_EXT");
		lua_pushinteger(lua, GL_STENCIL_INDEX1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX4_EXT");
		lua_pushinteger(lua, GL_STENCIL_INDEX4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX8_EXT");
		lua_pushinteger(lua, GL_STENCIL_INDEX8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_INDEX16_EXT");
		lua_pushinteger(lua, GL_STENCIL_INDEX16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_RED_SIZE_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_RED_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_GREEN_SIZE_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_GREEN_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_BLUE_SIZE_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_BLUE_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_ALPHA_SIZE_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_ALPHA_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_DEPTH_SIZE_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_DEPTH_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_STENCIL_SIZE_EXT");
		lua_pushinteger(lua, GL_RENDERBUFFER_STENCIL_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_framebuffer_sRGB");
		lua_pushinteger(lua, GL_EXT_framebuffer_sRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_SRGB_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_SRGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_SRGB_CAPABLE_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_SRGB_CAPABLE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_geometry_shader4");
		lua_pushinteger(lua, GL_EXT_geometry_shader4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINES_ADJACENCY_EXT");
		lua_pushinteger(lua, GL_LINES_ADJACENCY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LINE_STRIP_ADJACENCY_EXT");
		lua_pushinteger(lua, GL_LINE_STRIP_ADJACENCY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLES_ADJACENCY_EXT");
		lua_pushinteger(lua, GL_TRIANGLES_ADJACENCY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_STRIP_ADJACENCY_EXT");
		lua_pushinteger(lua, GL_TRIANGLE_STRIP_ADJACENCY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_POINT_SIZE_EXT");
		lua_pushinteger(lua, GL_PROGRAM_POINT_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VARYING_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_VARYING_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_ATTACHMENT_LAYERED_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_SHADER_EXT");
		lua_pushinteger(lua, GL_GEOMETRY_SHADER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_VERTICES_OUT_EXT");
		lua_pushinteger(lua, GL_GEOMETRY_VERTICES_OUT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_INPUT_TYPE_EXT");
		lua_pushinteger(lua, GL_GEOMETRY_INPUT_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_OUTPUT_TYPE_EXT");
		lua_pushinteger(lua, GL_GEOMETRY_OUTPUT_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_VARYING_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_VARYING_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_VARYING_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_OUTPUT_VERTICES_EXT");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_gpu_program_parameters");
		lua_pushinteger(lua, GL_EXT_gpu_program_parameters);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_gpu_shader4");
		lua_pushinteger(lua, GL_EXT_gpu_shader4);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_INTEGER_EXT");
//		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_SAMPLER_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_SAMPLER_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_BUFFER_EXT");
		lua_pushinteger(lua, GL_SAMPLER_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_1D_ARRAY_SHADOW_EXT");
		lua_pushinteger(lua, GL_SAMPLER_1D_ARRAY_SHADOW_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_2D_ARRAY_SHADOW_EXT");
		lua_pushinteger(lua, GL_SAMPLER_2D_ARRAY_SHADOW_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_CUBE_SHADOW_EXT");
		lua_pushinteger(lua, GL_SAMPLER_CUBE_SHADOW_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_VEC2_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_VEC2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_VEC3_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_VEC3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_VEC4_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_VEC4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_1D_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_3D_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_CUBE_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_CUBE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_RECT_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_RECT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_BUFFER_EXT");
		lua_pushinteger(lua, GL_INT_SAMPLER_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_1D_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_3D_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_CUBE_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_CUBE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_RECT_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_BUFFER_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_histogram");
		lua_pushinteger(lua, GL_EXT_histogram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_HISTOGRAM_EXT");
		lua_pushinteger(lua, GL_PROXY_HISTOGRAM_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_WIDTH_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_WIDTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_FORMAT_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_FORMAT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_RED_SIZE_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_RED_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_GREEN_SIZE_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_GREEN_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_BLUE_SIZE_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_BLUE_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_ALPHA_SIZE_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_ALPHA_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_LUMINANCE_SIZE_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_LUMINANCE_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HISTOGRAM_SINK_EXT");
		lua_pushinteger(lua, GL_HISTOGRAM_SINK_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_EXT");
		lua_pushinteger(lua, GL_MINMAX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_FORMAT_EXT");
		lua_pushinteger(lua, GL_MINMAX_FORMAT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MINMAX_SINK_EXT");
		lua_pushinteger(lua, GL_MINMAX_SINK_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_index_array_formats");
		lua_pushinteger(lua, GL_EXT_index_array_formats);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_index_func");
		lua_pushinteger(lua, GL_EXT_index_func);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_index_material");
		lua_pushinteger(lua, GL_EXT_index_material);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_index_texture");
		lua_pushinteger(lua, GL_EXT_index_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_light_texture");
		lua_pushinteger(lua, GL_EXT_light_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_MATERIAL_EXT");
		lua_pushinteger(lua, GL_FRAGMENT_MATERIAL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_NORMAL_EXT");
		lua_pushinteger(lua, GL_FRAGMENT_NORMAL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_COLOR_EXT");
		lua_pushinteger(lua, GL_FRAGMENT_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTENUATION_EXT");
		lua_pushinteger(lua, GL_ATTENUATION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADOW_ATTENUATION_EXT");
		lua_pushinteger(lua, GL_SHADOW_ATTENUATION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_APPLICATION_MODE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_APPLICATION_MODE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LIGHT_EXT");
		lua_pushinteger(lua, GL_TEXTURE_LIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MATERIAL_FACE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_MATERIAL_FACE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MATERIAL_PARAMETER_EXT");
		lua_pushinteger(lua, GL_TEXTURE_MATERIAL_PARAMETER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_misc_attribute");
		lua_pushinteger(lua, GL_EXT_misc_attribute);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_multi_draw_arrays");
		lua_pushinteger(lua, GL_EXT_multi_draw_arrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_multisample");
		lua_pushinteger(lua, GL_EXT_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_EXT");
		lua_pushinteger(lua, GL_MULTISAMPLE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_MASK_EXT");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_MASK_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_ONE_EXT");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_ONE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_EXT");
		lua_pushinteger(lua, GL_SAMPLE_MASK_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "1PASS_EXT");
		lua_pushinteger(lua, GL_1PASS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2PASS_0_EXT");
		lua_pushinteger(lua, GL_2PASS_0_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2PASS_1_EXT");
		lua_pushinteger(lua, GL_2PASS_1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_0_EXT");
		lua_pushinteger(lua, GL_4PASS_0_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_1_EXT");
		lua_pushinteger(lua, GL_4PASS_1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_2_EXT");
		lua_pushinteger(lua, GL_4PASS_2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_3_EXT");
		lua_pushinteger(lua, GL_4PASS_3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_BUFFERS_EXT");
		lua_pushinteger(lua, GL_SAMPLE_BUFFERS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES_EXT");
		lua_pushinteger(lua, GL_SAMPLES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_VALUE_EXT");
		lua_pushinteger(lua, GL_SAMPLE_MASK_VALUE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_INVERT_EXT");
		lua_pushinteger(lua, GL_SAMPLE_MASK_INVERT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_PATTERN_EXT");
		lua_pushinteger(lua, GL_SAMPLE_PATTERN_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_BIT_EXT");
		lua_pushinteger(lua, GL_MULTISAMPLE_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_packed_depth_stencil");
		lua_pushinteger(lua, GL_EXT_packed_depth_stencil);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_STENCIL_EXT");
		lua_pushinteger(lua, GL_DEPTH_STENCIL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_24_8_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_24_8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH24_STENCIL8_EXT");
		lua_pushinteger(lua, GL_DEPTH24_STENCIL8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_STENCIL_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_STENCIL_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_packed_float");
		lua_pushinteger(lua, GL_EXT_packed_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R11F_G11F_B10F_EXT");
		lua_pushinteger(lua, GL_R11F_G11F_B10F_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_10F_11F_11F_REV_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_10F_11F_11F_REV_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_SIGNED_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_RGBA_SIGNED_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_packed_pixels");
		lua_pushinteger(lua, GL_EXT_packed_pixels);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_BYTE_3_3_2_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_BYTE_3_3_2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_4_4_4_4_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_4_4_4_4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_5_5_5_1_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_5_5_5_1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_8_8_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_8_8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_10_10_10_2_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_10_10_10_2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_paletted_texture");
		lua_pushinteger(lua, GL_EXT_paletted_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D");
		lua_pushinteger(lua, GL_TEXTURE_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D");
		lua_pushinteger(lua, GL_TEXTURE_2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_FORMAT_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_FORMAT_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_WIDTH_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_WIDTH_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_RED_SIZE_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_RED_SIZE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_GREEN_SIZE_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_GREEN_SIZE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_BLUE_SIZE_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_BLUE_SIZE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_ALPHA_SIZE_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_ALPHA_SIZE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_LUMINANCE_SIZE_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_LUMINANCE_SIZE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLOR_TABLE_INTENSITY_SIZE_EXT");
//		lua_pushinteger(lua, GL_COLOR_TABLE_INTENSITY_SIZE_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX1_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX2_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX4_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX8_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX12_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_INDEX16_EXT");
		lua_pushinteger(lua, GL_COLOR_INDEX16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INDEX_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_INDEX_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_ARB");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP_ARB");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP_ARB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_pixel_buffer_object");
		lua_pushinteger(lua, GL_EXT_pixel_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_PACK_BUFFER_EXT");
		lua_pushinteger(lua, GL_PIXEL_PACK_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_UNPACK_BUFFER_EXT");
		lua_pushinteger(lua, GL_PIXEL_UNPACK_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_PACK_BUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_PIXEL_PACK_BUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_UNPACK_BUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_PIXEL_UNPACK_BUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_pixel_transform");
		lua_pushinteger(lua, GL_EXT_pixel_transform);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_TRANSFORM_2D_EXT");
		lua_pushinteger(lua, GL_PIXEL_TRANSFORM_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MAG_FILTER_EXT");
		lua_pushinteger(lua, GL_PIXEL_MAG_FILTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_MIN_FILTER_EXT");
		lua_pushinteger(lua, GL_PIXEL_MIN_FILTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_CUBIC_WEIGHT_EXT");
		lua_pushinteger(lua, GL_PIXEL_CUBIC_WEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CUBIC_EXT");
		lua_pushinteger(lua, GL_CUBIC_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AVERAGE_EXT");
		lua_pushinteger(lua, GL_AVERAGE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT");
		lua_pushinteger(lua, GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT");
		lua_pushinteger(lua, GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_TRANSFORM_2D_MATRIX_EXT");
		lua_pushinteger(lua, GL_PIXEL_TRANSFORM_2D_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_pixel_transform_color_table");
		lua_pushinteger(lua, GL_EXT_pixel_transform_color_table);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_point_parameters");
		lua_pushinteger(lua, GL_EXT_point_parameters);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_MIN_EXT");
		lua_pushinteger(lua, GL_POINT_SIZE_MIN_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SIZE_MAX_EXT");
		lua_pushinteger(lua, GL_POINT_SIZE_MAX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_FADE_THRESHOLD_SIZE_EXT");
		lua_pushinteger(lua, GL_POINT_FADE_THRESHOLD_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DISTANCE_ATTENUATION_EXT");
		lua_pushinteger(lua, GL_DISTANCE_ATTENUATION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_polygon_offset");
		lua_pushinteger(lua, GL_EXT_polygon_offset);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_EXT");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_FACTOR_EXT");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_FACTOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POLYGON_OFFSET_BIAS_EXT");
		lua_pushinteger(lua, GL_POLYGON_OFFSET_BIAS_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_polygon_offset_clamp");
//		lua_pushinteger(lua, GL_EXT_polygon_offset_clamp);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "POLYGON_OFFSET_CLAMP_EXT");
//		lua_pushinteger(lua, GL_POLYGON_OFFSET_CLAMP_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_post_depth_coverage");
//		lua_pushinteger(lua, GL_EXT_post_depth_coverage);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_provoking_vertex");
		lua_pushinteger(lua, GL_EXT_provoking_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT");
		lua_pushinteger(lua, GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIRST_VERTEX_CONVENTION_EXT");
		lua_pushinteger(lua, GL_FIRST_VERTEX_CONVENTION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LAST_VERTEX_CONVENTION_EXT");
		lua_pushinteger(lua, GL_LAST_VERTEX_CONVENTION_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROVOKING_VERTEX_EXT");
		lua_pushinteger(lua, GL_PROVOKING_VERTEX_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_raster_multisample");
//		lua_pushinteger(lua, GL_EXT_raster_multisample);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SAMPLES_NV");
		lua_pushinteger(lua, GL_COLOR_SAMPLES_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "RASTER_MULTISAMPLE_EXT");
//		lua_pushinteger(lua, GL_RASTER_MULTISAMPLE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RASTER_SAMPLES_EXT");
//		lua_pushinteger(lua, GL_RASTER_SAMPLES_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_RASTER_SAMPLES_EXT");
//		lua_pushinteger(lua, GL_MAX_RASTER_SAMPLES_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RASTER_FIXED_SAMPLE_LOCATIONS_EXT");
//		lua_pushinteger(lua, GL_RASTER_FIXED_SAMPLE_LOCATIONS_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MULTISAMPLE_RASTERIZATION_ALLOWED_EXT");
//		lua_pushinteger(lua, GL_MULTISAMPLE_RASTERIZATION_ALLOWED_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EFFECTIVE_RASTER_SAMPLES_EXT");
//		lua_pushinteger(lua, GL_EFFECTIVE_RASTER_SAMPLES_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEPTH_SAMPLES_NV");
//		lua_pushinteger(lua, GL_DEPTH_SAMPLES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STENCIL_SAMPLES_NV");
//		lua_pushinteger(lua, GL_STENCIL_SAMPLES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIXED_DEPTH_SAMPLES_SUPPORTED_NV");
//		lua_pushinteger(lua, GL_MIXED_DEPTH_SAMPLES_SUPPORTED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIXED_STENCIL_SAMPLES_SUPPORTED_NV");
//		lua_pushinteger(lua, GL_MIXED_STENCIL_SAMPLES_SUPPORTED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COVERAGE_MODULATION_TABLE_NV");
//		lua_pushinteger(lua, GL_COVERAGE_MODULATION_TABLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COVERAGE_MODULATION_NV");
//		lua_pushinteger(lua, GL_COVERAGE_MODULATION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COVERAGE_MODULATION_TABLE_SIZE_NV");
//		lua_pushinteger(lua, GL_COVERAGE_MODULATION_TABLE_SIZE_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_rescale_normal");
		lua_pushinteger(lua, GL_EXT_rescale_normal);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESCALE_NORMAL_EXT");
		lua_pushinteger(lua, GL_RESCALE_NORMAL_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_scene_marker");
//		lua_pushinteger(lua, GL_EXT_scene_marker);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_secondary_color");
		lua_pushinteger(lua, GL_EXT_secondary_color);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SUM_EXT");
		lua_pushinteger(lua, GL_COLOR_SUM_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_SECONDARY_COLOR_EXT");
		lua_pushinteger(lua, GL_CURRENT_SECONDARY_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_SIZE_EXT");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_EXT");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_separate_shader_objects");
		lua_pushinteger(lua, GL_EXT_separate_shader_objects);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_PROGRAM_EXT");
		lua_pushinteger(lua, GL_ACTIVE_PROGRAM_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_separate_specular_color");
		lua_pushinteger(lua, GL_EXT_separate_specular_color);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LIGHT_MODEL_COLOR_CONTROL_EXT");
		lua_pushinteger(lua, GL_LIGHT_MODEL_COLOR_CONTROL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SINGLE_COLOR_EXT");
		lua_pushinteger(lua, GL_SINGLE_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARATE_SPECULAR_COLOR_EXT");
		lua_pushinteger(lua, GL_SEPARATE_SPECULAR_COLOR_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_shader_image_load_formatted");
//		lua_pushinteger(lua, GL_EXT_shader_image_load_formatted);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_shader_image_load_store");
		lua_pushinteger(lua, GL_EXT_shader_image_load_store);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNIFORM_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_UNIFORM_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_FETCH_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_TEXTURE_FETCH_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMMAND_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_COMMAND_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_BUFFER_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_PIXEL_BUFFER_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_UPDATE_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_TEXTURE_UPDATE_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_UPDATE_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_BUFFER_UPDATE_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAMEBUFFER_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_FRAMEBUFFER_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATOMIC_COUNTER_BARRIER_BIT_EXT");
		lua_pushinteger(lua, GL_ATOMIC_COUNTER_BARRIER_BIT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_IMAGE_UNITS_EXT");
		lua_pushinteger(lua, GL_MAX_IMAGE_UNITS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT");
		lua_pushinteger(lua, GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BINDING_NAME_EXT");
		lua_pushinteger(lua, GL_IMAGE_BINDING_NAME_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BINDING_LEVEL_EXT");
		lua_pushinteger(lua, GL_IMAGE_BINDING_LEVEL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BINDING_LAYERED_EXT");
		lua_pushinteger(lua, GL_IMAGE_BINDING_LAYERED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BINDING_LAYER_EXT");
		lua_pushinteger(lua, GL_IMAGE_BINDING_LAYER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BINDING_ACCESS_EXT");
		lua_pushinteger(lua, GL_IMAGE_BINDING_ACCESS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_1D_EXT");
		lua_pushinteger(lua, GL_IMAGE_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_2D_EXT");
		lua_pushinteger(lua, GL_IMAGE_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_3D_EXT");
		lua_pushinteger(lua, GL_IMAGE_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_2D_RECT_EXT");
		lua_pushinteger(lua, GL_IMAGE_2D_RECT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_CUBE_EXT");
		lua_pushinteger(lua, GL_IMAGE_CUBE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BUFFER_EXT");
		lua_pushinteger(lua, GL_IMAGE_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_IMAGE_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_IMAGE_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_CUBE_MAP_ARRAY_EXT");
		lua_pushinteger(lua, GL_IMAGE_CUBE_MAP_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_2D_MULTISAMPLE_EXT");
		lua_pushinteger(lua, GL_IMAGE_2D_MULTISAMPLE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_2D_MULTISAMPLE_ARRAY_EXT");
		lua_pushinteger(lua, GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_1D_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_2D_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_3D_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_2D_RECT_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_2D_RECT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_CUBE_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_CUBE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_BUFFER_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_CUBE_MAP_ARRAY_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_2D_MULTISAMPLE_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_2D_MULTISAMPLE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT");
		lua_pushinteger(lua, GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_1D_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_3D_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_RECT_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_CUBE_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_CUBE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_BUFFER_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_IMAGE_SAMPLES_EXT");
		lua_pushinteger(lua, GL_MAX_IMAGE_SAMPLES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMAGE_BINDING_FORMAT_EXT");
		lua_pushinteger(lua, GL_IMAGE_BINDING_FORMAT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALL_BARRIER_BITS_EXT");
		lua_pushinteger(lua, GL_ALL_BARRIER_BITS_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_shader_integer_mix");
//		lua_pushinteger(lua, GL_EXT_shader_integer_mix);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_shadow_funcs");
		lua_pushinteger(lua, GL_EXT_shadow_funcs);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_shared_texture_palette");
		lua_pushinteger(lua, GL_EXT_shared_texture_palette);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHARED_TEXTURE_PALETTE_EXT");
		lua_pushinteger(lua, GL_SHARED_TEXTURE_PALETTE_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_sparse_texture2");
//		lua_pushinteger(lua, GL_EXT_sparse_texture2);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_stencil_clear_tag");
		lua_pushinteger(lua, GL_EXT_stencil_clear_tag);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_TAG_BITS_EXT");
		lua_pushinteger(lua, GL_STENCIL_TAG_BITS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_CLEAR_TAG_VALUE_EXT");
		lua_pushinteger(lua, GL_STENCIL_CLEAR_TAG_VALUE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_stencil_two_side");
		lua_pushinteger(lua, GL_EXT_stencil_two_side);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STENCIL_TEST_TWO_SIDE_EXT");
		lua_pushinteger(lua, GL_STENCIL_TEST_TWO_SIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_STENCIL_FACE_EXT");
		lua_pushinteger(lua, GL_ACTIVE_STENCIL_FACE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_stencil_wrap");
		lua_pushinteger(lua, GL_EXT_stencil_wrap);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INCR_WRAP_EXT");
		lua_pushinteger(lua, GL_INCR_WRAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DECR_WRAP_EXT");
		lua_pushinteger(lua, GL_DECR_WRAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_subtexture");
		lua_pushinteger(lua, GL_EXT_subtexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture");
		lua_pushinteger(lua, GL_EXT_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA4_EXT");
		lua_pushinteger(lua, GL_ALPHA4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA8_EXT");
		lua_pushinteger(lua, GL_ALPHA8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA12_EXT");
		lua_pushinteger(lua, GL_ALPHA12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16_EXT");
		lua_pushinteger(lua, GL_ALPHA16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE4_EXT");
		lua_pushinteger(lua, GL_LUMINANCE4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8_EXT");
		lua_pushinteger(lua, GL_LUMINANCE8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_EXT");
		lua_pushinteger(lua, GL_LUMINANCE12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16_EXT");
		lua_pushinteger(lua, GL_LUMINANCE16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE4_ALPHA4_EXT");
		lua_pushinteger(lua, GL_LUMINANCE4_ALPHA4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE6_ALPHA2_EXT");
		lua_pushinteger(lua, GL_LUMINANCE6_ALPHA2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8_ALPHA8_EXT");
		lua_pushinteger(lua, GL_LUMINANCE8_ALPHA8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_ALPHA4_EXT");
		lua_pushinteger(lua, GL_LUMINANCE12_ALPHA4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE12_ALPHA12_EXT");
		lua_pushinteger(lua, GL_LUMINANCE12_ALPHA12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16_ALPHA16_EXT");
		lua_pushinteger(lua, GL_LUMINANCE16_ALPHA16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY_EXT");
		lua_pushinteger(lua, GL_INTENSITY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY4_EXT");
		lua_pushinteger(lua, GL_INTENSITY4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY8_EXT");
		lua_pushinteger(lua, GL_INTENSITY8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY12_EXT");
		lua_pushinteger(lua, GL_INTENSITY12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16_EXT");
		lua_pushinteger(lua, GL_INTENSITY16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB2_EXT");
		lua_pushinteger(lua, GL_RGB2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB4_EXT");
		lua_pushinteger(lua, GL_RGB4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB5_EXT");
		lua_pushinteger(lua, GL_RGB5_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8_EXT");
		lua_pushinteger(lua, GL_RGB8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10_EXT");
		lua_pushinteger(lua, GL_RGB10_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB12_EXT");
		lua_pushinteger(lua, GL_RGB12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16_EXT");
		lua_pushinteger(lua, GL_RGB16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA2_EXT");
		lua_pushinteger(lua, GL_RGBA2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA4_EXT");
		lua_pushinteger(lua, GL_RGBA4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB5_A1_EXT");
		lua_pushinteger(lua, GL_RGB5_A1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8_EXT");
		lua_pushinteger(lua, GL_RGBA8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB10_A2_EXT");
		lua_pushinteger(lua, GL_RGB10_A2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA12_EXT");
		lua_pushinteger(lua, GL_RGBA12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16_EXT");
		lua_pushinteger(lua, GL_RGBA16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RED_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_RED_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GREEN_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_GREEN_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BLUE_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BLUE_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_ALPHA_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_ALPHA_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LUMINANCE_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_LUMINANCE_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_INTENSITY_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_INTENSITY_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACE_EXT");
		lua_pushinteger(lua, GL_REPLACE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D_EXT");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D_EXT");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture3D");
		lua_pushinteger(lua, GL_EXT_texture3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_SKIP_IMAGES_EXT");
		lua_pushinteger(lua, GL_PACK_SKIP_IMAGES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_IMAGE_HEIGHT_EXT");
		lua_pushinteger(lua, GL_PACK_IMAGE_HEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_SKIP_IMAGES_EXT");
		lua_pushinteger(lua, GL_UNPACK_SKIP_IMAGES_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_IMAGE_HEIGHT_EXT");
		lua_pushinteger(lua, GL_UNPACK_IMAGE_HEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_3D_EXT");
		lua_pushinteger(lua, GL_TEXTURE_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_3D_EXT");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_3D_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DEPTH_EXT");
		lua_pushinteger(lua, GL_TEXTURE_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_WRAP_R_EXT");
		lua_pushinteger(lua, GL_TEXTURE_WRAP_R_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_3D_TEXTURE_SIZE_EXT");
		lua_pushinteger(lua, GL_MAX_3D_TEXTURE_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_array");
		lua_pushinteger(lua, GL_EXT_texture_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPARE_REF_DEPTH_TO_TEXTURE_EXT");
		lua_pushinteger(lua, GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ARRAY_TEXTURE_LAYERS_EXT");
		lua_pushinteger(lua, GL_MAX_ARRAY_TEXTURE_LAYERS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_1D_ARRAY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_1D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_2D_ARRAY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_2D_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_buffer_object");
		lua_pushinteger(lua, GL_EXT_texture_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_BUFFER_SIZE_EXT");
		lua_pushinteger(lua, GL_MAX_TEXTURE_BUFFER_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_BUFFER_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_DATA_STORE_BINDING_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BUFFER_FORMAT_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BUFFER_FORMAT_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_compression_dxt1");
//		lua_pushinteger(lua, GL_EXT_texture_compression_dxt1);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_compression_latc");
		lua_pushinteger(lua, GL_EXT_texture_compression_latc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE_LATC1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_LATC1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_compression_rgtc");
		lua_pushinteger(lua, GL_EXT_texture_compression_rgtc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RED_RGTC1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_RED_RGTC1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SIGNED_RED_RGTC1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_RED_RGTC1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RED_GREEN_RGTC2_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_RED_GREEN_RGTC2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_compression_s3tc");
//		lua_pushinteger(lua, GL_EXT_texture_compression_s3tc);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGB_S3TC_DXT1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT3_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_RGBA_S3TC_DXT5_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_cube_map");
//		lua_pushinteger(lua, GL_EXT_texture_cube_map);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_MAP_EXT");
		lua_pushinteger(lua, GL_NORMAL_MAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REFLECTION_MAP_EXT");
		lua_pushinteger(lua, GL_REFLECTION_MAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_CUBE_MAP_EXT");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_CUBE_MAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_X_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_X_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Y_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_POSITIVE_Z_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT");
		lua_pushinteger(lua, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_CUBE_MAP_EXT");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_CUBE_MAP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_CUBE_MAP_TEXTURE_SIZE_EXT");
		lua_pushinteger(lua, GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_edge_clamp");
//		lua_pushinteger(lua, GL_EXT_texture_edge_clamp);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLAMP_TO_EDGE_EXT");
//		lua_pushinteger(lua, GL_CLAMP_TO_EDGE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_env");
//		lua_pushinteger(lua, GL_EXT_texture_env);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_env_add");
		lua_pushinteger(lua, GL_EXT_texture_env_add);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_env_combine");
		lua_pushinteger(lua, GL_EXT_texture_env_combine);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_EXT");
		lua_pushinteger(lua, GL_COMBINE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_RGB_EXT");
		lua_pushinteger(lua, GL_COMBINE_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE_ALPHA_EXT");
		lua_pushinteger(lua, GL_COMBINE_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_SCALE_EXT");
		lua_pushinteger(lua, GL_RGB_SCALE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ADD_SIGNED_EXT");
		lua_pushinteger(lua, GL_ADD_SIGNED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERPOLATE_EXT");
		lua_pushinteger(lua, GL_INTERPOLATE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_EXT");
		lua_pushinteger(lua, GL_CONSTANT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR_EXT");
		lua_pushinteger(lua, GL_PRIMARY_COLOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PREVIOUS_EXT");
		lua_pushinteger(lua, GL_PREVIOUS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_RGB_EXT");
		lua_pushinteger(lua, GL_SOURCE0_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_RGB_EXT");
		lua_pushinteger(lua, GL_SOURCE1_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_RGB_EXT");
		lua_pushinteger(lua, GL_SOURCE2_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE0_ALPHA_EXT");
		lua_pushinteger(lua, GL_SOURCE0_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE1_ALPHA_EXT");
		lua_pushinteger(lua, GL_SOURCE1_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE2_ALPHA_EXT");
		lua_pushinteger(lua, GL_SOURCE2_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_RGB_EXT");
		lua_pushinteger(lua, GL_OPERAND0_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_RGB_EXT");
		lua_pushinteger(lua, GL_OPERAND1_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_RGB_EXT");
		lua_pushinteger(lua, GL_OPERAND2_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND0_ALPHA_EXT");
		lua_pushinteger(lua, GL_OPERAND0_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND1_ALPHA_EXT");
		lua_pushinteger(lua, GL_OPERAND1_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND2_ALPHA_EXT");
		lua_pushinteger(lua, GL_OPERAND2_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_env_dot3");
		lua_pushinteger(lua, GL_EXT_texture_env_dot3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGB_EXT");
		lua_pushinteger(lua, GL_DOT3_RGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT3_RGBA_EXT");
		lua_pushinteger(lua, GL_DOT3_RGBA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_filter_anisotropic");
		lua_pushinteger(lua, GL_EXT_texture_filter_anisotropic);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_ANISOTROPY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_MAX_ANISOTROPY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_MAX_ANISOTROPY_EXT");
		lua_pushinteger(lua, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_filter_minmax");
//		lua_pushinteger(lua, GL_EXT_texture_filter_minmax);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_REDUCTION_MODE_EXT");
//		lua_pushinteger(lua, GL_TEXTURE_REDUCTION_MODE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "WEIGHTED_AVERAGE_EXT");
//		lua_pushinteger(lua, GL_WEIGHTED_AVERAGE_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_integer");
		lua_pushinteger(lua, GL_EXT_texture_integer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA32UI_EXT");
		lua_pushinteger(lua, GL_RGBA32UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB32UI_EXT");
		lua_pushinteger(lua, GL_RGB32UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA32UI_EXT");
		lua_pushinteger(lua, GL_ALPHA32UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY32UI_EXT");
		lua_pushinteger(lua, GL_INTENSITY32UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE32UI_EXT");
		lua_pushinteger(lua, GL_LUMINANCE32UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA32UI_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA32UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16UI_EXT");
		lua_pushinteger(lua, GL_RGBA16UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16UI_EXT");
		lua_pushinteger(lua, GL_RGB16UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16UI_EXT");
		lua_pushinteger(lua, GL_ALPHA16UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16UI_EXT");
		lua_pushinteger(lua, GL_INTENSITY16UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16UI_EXT");
		lua_pushinteger(lua, GL_LUMINANCE16UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA16UI_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA16UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8UI_EXT");
		lua_pushinteger(lua, GL_RGBA8UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8UI_EXT");
		lua_pushinteger(lua, GL_RGB8UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA8UI_EXT");
		lua_pushinteger(lua, GL_ALPHA8UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY8UI_EXT");
		lua_pushinteger(lua, GL_INTENSITY8UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8UI_EXT");
		lua_pushinteger(lua, GL_LUMINANCE8UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA8UI_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA8UI_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA32I_EXT");
		lua_pushinteger(lua, GL_RGBA32I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB32I_EXT");
		lua_pushinteger(lua, GL_RGB32I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA32I_EXT");
		lua_pushinteger(lua, GL_ALPHA32I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY32I_EXT");
		lua_pushinteger(lua, GL_INTENSITY32I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE32I_EXT");
		lua_pushinteger(lua, GL_LUMINANCE32I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA32I_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA32I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16I_EXT");
		lua_pushinteger(lua, GL_RGBA16I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16I_EXT");
		lua_pushinteger(lua, GL_RGB16I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16I_EXT");
		lua_pushinteger(lua, GL_ALPHA16I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16I_EXT");
		lua_pushinteger(lua, GL_INTENSITY16I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16I_EXT");
		lua_pushinteger(lua, GL_LUMINANCE16I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA16I_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA16I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8I_EXT");
		lua_pushinteger(lua, GL_RGBA8I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8I_EXT");
		lua_pushinteger(lua, GL_RGB8I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA8I_EXT");
		lua_pushinteger(lua, GL_ALPHA8I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY8I_EXT");
		lua_pushinteger(lua, GL_INTENSITY8I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8I_EXT");
		lua_pushinteger(lua, GL_LUMINANCE8I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA8I_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA8I_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_INTEGER_EXT");
		lua_pushinteger(lua, GL_RED_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_INTEGER_EXT");
		lua_pushinteger(lua, GL_GREEN_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_INTEGER_EXT");
		lua_pushinteger(lua, GL_BLUE_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_INTEGER_EXT");
		lua_pushinteger(lua, GL_ALPHA_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_INTEGER_EXT");
		lua_pushinteger(lua, GL_RGB_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_INTEGER_EXT");
		lua_pushinteger(lua, GL_RGBA_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGR_INTEGER_EXT");
		lua_pushinteger(lua, GL_BGR_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA_INTEGER_EXT");
		lua_pushinteger(lua, GL_BGRA_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_INTEGER_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA_INTEGER_EXT");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_INTEGER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_INTEGER_MODE_EXT");
		lua_pushinteger(lua, GL_RGBA_INTEGER_MODE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_lod_bias");
		lua_pushinteger(lua, GL_EXT_texture_lod_bias);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_LOD_BIAS_EXT");
		lua_pushinteger(lua, GL_MAX_TEXTURE_LOD_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_FILTER_CONTROL_EXT");
		lua_pushinteger(lua, GL_TEXTURE_FILTER_CONTROL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LOD_BIAS_EXT");
		lua_pushinteger(lua, GL_TEXTURE_LOD_BIAS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_mirror_clamp");
		lua_pushinteger(lua, GL_EXT_texture_mirror_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRROR_CLAMP_EXT");
		lua_pushinteger(lua, GL_MIRROR_CLAMP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRROR_CLAMP_TO_EDGE_EXT");
		lua_pushinteger(lua, GL_MIRROR_CLAMP_TO_EDGE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRROR_CLAMP_TO_BORDER_EXT");
		lua_pushinteger(lua, GL_MIRROR_CLAMP_TO_BORDER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_object");
		lua_pushinteger(lua, GL_EXT_texture_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_PRIORITY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_PRIORITY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RESIDENT_EXT");
		lua_pushinteger(lua, GL_TEXTURE_RESIDENT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D_BINDING_EXT");
		lua_pushinteger(lua, GL_TEXTURE_1D_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_BINDING_EXT");
		lua_pushinteger(lua, GL_TEXTURE_2D_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_3D_BINDING_EXT");
		lua_pushinteger(lua, GL_TEXTURE_3D_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_perturb_normal");
		lua_pushinteger(lua, GL_EXT_texture_perturb_normal);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PERTURB_EXT");
		lua_pushinteger(lua, GL_PERTURB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_NORMAL_EXT");
		lua_pushinteger(lua, GL_TEXTURE_NORMAL_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_rectangle");
//		lua_pushinteger(lua, GL_EXT_texture_rectangle);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_RECTANGLE_EXT");
//		lua_pushinteger(lua, GL_TEXTURE_RECTANGLE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_BINDING_RECTANGLE_EXT");
//		lua_pushinteger(lua, GL_TEXTURE_BINDING_RECTANGLE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROXY_TEXTURE_RECTANGLE_EXT");
//		lua_pushinteger(lua, GL_PROXY_TEXTURE_RECTANGLE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_RECTANGLE_TEXTURE_SIZE_EXT");
//		lua_pushinteger(lua, GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_sRGB");
		lua_pushinteger(lua, GL_EXT_texture_sRGB);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB_EXT");
		lua_pushinteger(lua, GL_SRGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB8_EXT");
		lua_pushinteger(lua, GL_SRGB8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB_ALPHA_EXT");
		lua_pushinteger(lua, GL_SRGB_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SRGB8_ALPHA8_EXT");
		lua_pushinteger(lua, GL_SRGB8_ALPHA8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE_ALPHA_EXT");
		lua_pushinteger(lua, GL_SLUMINANCE_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE8_ALPHA8_EXT");
		lua_pushinteger(lua, GL_SLUMINANCE8_ALPHA8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE_EXT");
		lua_pushinteger(lua, GL_SLUMINANCE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLUMINANCE8_EXT");
		lua_pushinteger(lua, GL_SLUMINANCE8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SLUMINANCE_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SLUMINANCE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SLUMINANCE_ALPHA_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SLUMINANCE_ALPHA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_S3TC_DXT1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT");
		lua_pushinteger(lua, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_texture_sRGB_decode");
//		lua_pushinteger(lua, GL_EXT_texture_sRGB_decode);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_SRGB_DECODE_EXT");
//		lua_pushinteger(lua, GL_TEXTURE_SRGB_DECODE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DECODE_EXT");
//		lua_pushinteger(lua, GL_DECODE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SKIP_DECODE_EXT");
//		lua_pushinteger(lua, GL_SKIP_DECODE_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_shared_exponent");
		lua_pushinteger(lua, GL_EXT_texture_shared_exponent);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB9_E5_EXT");
		lua_pushinteger(lua, GL_RGB9_E5_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_5_9_9_9_REV_EXT");
		lua_pushinteger(lua, GL_UNSIGNED_INT_5_9_9_9_REV_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SHARED_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_SHARED_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_snorm");
		lua_pushinteger(lua, GL_EXT_texture_snorm);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_SNORM");
		lua_pushinteger(lua, GL_RED_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG_SNORM");
		lua_pushinteger(lua, GL_RG_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_SNORM");
		lua_pushinteger(lua, GL_RGB_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_SNORM");
		lua_pushinteger(lua, GL_RGBA_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R8_SNORM");
		lua_pushinteger(lua, GL_R8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG8_SNORM");
		lua_pushinteger(lua, GL_RG8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB8_SNORM");
		lua_pushinteger(lua, GL_RGB8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA8_SNORM");
		lua_pushinteger(lua, GL_RGBA8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R16_SNORM");
		lua_pushinteger(lua, GL_R16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RG16_SNORM");
		lua_pushinteger(lua, GL_RG16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB16_SNORM");
		lua_pushinteger(lua, GL_RGB16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA16_SNORM");
		lua_pushinteger(lua, GL_RGBA16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_NORMALIZED");
		lua_pushinteger(lua, GL_SIGNED_NORMALIZED);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_SNORM");
		lua_pushinteger(lua, GL_ALPHA_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_SNORM");
		lua_pushinteger(lua, GL_LUMINANCE_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE_ALPHA_SNORM");
		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY_SNORM");
		lua_pushinteger(lua, GL_INTENSITY_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA8_SNORM");
		lua_pushinteger(lua, GL_ALPHA8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8_SNORM");
		lua_pushinteger(lua, GL_LUMINANCE8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE8_ALPHA8_SNORM");
		lua_pushinteger(lua, GL_LUMINANCE8_ALPHA8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY8_SNORM");
		lua_pushinteger(lua, GL_INTENSITY8_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA16_SNORM");
		lua_pushinteger(lua, GL_ALPHA16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16_SNORM");
		lua_pushinteger(lua, GL_LUMINANCE16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LUMINANCE16_ALPHA16_SNORM");
		lua_pushinteger(lua, GL_LUMINANCE16_ALPHA16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTENSITY16_SNORM");
		lua_pushinteger(lua, GL_INTENSITY16_SNORM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_texture_swizzle");
		lua_pushinteger(lua, GL_EXT_texture_swizzle);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_R_EXT");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_R_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_G_EXT");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_G_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_B_EXT");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_B_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_A_EXT");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_A_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SWIZZLE_RGBA_EXT");
		lua_pushinteger(lua, GL_TEXTURE_SWIZZLE_RGBA_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_timer_query");
		lua_pushinteger(lua, GL_EXT_timer_query);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TIME_ELAPSED_EXT");
		lua_pushinteger(lua, GL_TIME_ELAPSED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_transform_feedback");
		lua_pushinteger(lua, GL_EXT_transform_feedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_MODE_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_VARYINGS_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_VARYINGS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_START_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVES_GENERATED_EXT");
		lua_pushinteger(lua, GL_PRIMITIVES_GENERATED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RASTERIZER_DISCARD_EXT");
		lua_pushinteger(lua, GL_RASTERIZER_DISCARD_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLEAVED_ATTRIBS_EXT");
		lua_pushinteger(lua, GL_INTERLEAVED_ATTRIBS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARATE_ATTRIBS_EXT");
		lua_pushinteger(lua, GL_SEPARATE_ATTRIBS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_vertex_array");
		lua_pushinteger(lua, GL_EXT_vertex_array);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "DOUBLE_EXT");
//		lua_pushinteger(lua, GL_DOUBLE_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_EXT");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_EXT");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_EXT");
		lua_pushinteger(lua, GL_COLOR_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_EXT");
		lua_pushinteger(lua, GL_INDEX_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_EXT");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_EXT");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_SIZE_EXT");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_COUNT_EXT");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_COUNT_EXT");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_SIZE_EXT");
		lua_pushinteger(lua, GL_COLOR_ARRAY_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_COLOR_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_COLOR_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_COUNT_EXT");
		lua_pushinteger(lua, GL_COLOR_ARRAY_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_INDEX_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_INDEX_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_COUNT_EXT");
		lua_pushinteger(lua, GL_INDEX_ARRAY_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_SIZE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_COUNT_EXT");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_COUNT_EXT");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_COUNT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_COLOR_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_INDEX_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_vertex_array_bgra");
		lua_pushinteger(lua, GL_EXT_vertex_array_bgra);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BGRA");
		lua_pushinteger(lua, GL_BGRA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_vertex_attrib_64bit");
		lua_pushinteger(lua, GL_EXT_vertex_attrib_64bit);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT2_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT3_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT4_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT2x3_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT2x3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT2x4_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT2x4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT3x2_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT3x2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT3x4_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT3x4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT4x2_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT4x2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_MAT4x3_EXT");
		lua_pushinteger(lua, GL_DOUBLE_MAT4x3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_VEC2_EXT");
		lua_pushinteger(lua, GL_DOUBLE_VEC2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_VEC3_EXT");
		lua_pushinteger(lua, GL_DOUBLE_VEC3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOUBLE_VEC4_EXT");
		lua_pushinteger(lua, GL_DOUBLE_VEC4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_vertex_shader");
		lua_pushinteger(lua, GL_EXT_vertex_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_BINDING_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_BINDING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_INDEX_EXT");
		lua_pushinteger(lua, GL_OP_INDEX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_NEGATE_EXT");
		lua_pushinteger(lua, GL_OP_NEGATE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_DOT3_EXT");
		lua_pushinteger(lua, GL_OP_DOT3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_DOT4_EXT");
		lua_pushinteger(lua, GL_OP_DOT4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_MUL_EXT");
		lua_pushinteger(lua, GL_OP_MUL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_ADD_EXT");
		lua_pushinteger(lua, GL_OP_ADD_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_MADD_EXT");
		lua_pushinteger(lua, GL_OP_MADD_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_FRAC_EXT");
		lua_pushinteger(lua, GL_OP_FRAC_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_MAX_EXT");
		lua_pushinteger(lua, GL_OP_MAX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_MIN_EXT");
		lua_pushinteger(lua, GL_OP_MIN_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_SET_GE_EXT");
		lua_pushinteger(lua, GL_OP_SET_GE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_SET_LT_EXT");
		lua_pushinteger(lua, GL_OP_SET_LT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_CLAMP_EXT");
		lua_pushinteger(lua, GL_OP_CLAMP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_FLOOR_EXT");
		lua_pushinteger(lua, GL_OP_FLOOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_ROUND_EXT");
		lua_pushinteger(lua, GL_OP_ROUND_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_EXP_BASE_2_EXT");
		lua_pushinteger(lua, GL_OP_EXP_BASE_2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_LOG_BASE_2_EXT");
		lua_pushinteger(lua, GL_OP_LOG_BASE_2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_POWER_EXT");
		lua_pushinteger(lua, GL_OP_POWER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_RECIP_EXT");
		lua_pushinteger(lua, GL_OP_RECIP_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_RECIP_SQRT_EXT");
		lua_pushinteger(lua, GL_OP_RECIP_SQRT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_SUB_EXT");
		lua_pushinteger(lua, GL_OP_SUB_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_CROSS_PRODUCT_EXT");
		lua_pushinteger(lua, GL_OP_CROSS_PRODUCT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_MULTIPLY_MATRIX_EXT");
		lua_pushinteger(lua, GL_OP_MULTIPLY_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OP_MOV_EXT");
		lua_pushinteger(lua, GL_OP_MOV_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_VERTEX_EXT");
		lua_pushinteger(lua, GL_OUTPUT_VERTEX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_COLOR0_EXT");
		lua_pushinteger(lua, GL_OUTPUT_COLOR0_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_COLOR1_EXT");
		lua_pushinteger(lua, GL_OUTPUT_COLOR1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD0_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD0_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD1_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD2_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD2_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD3_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD3_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD4_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD4_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD5_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD5_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD6_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD6_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD7_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD7_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD8_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD8_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD9_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD9_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD10_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD10_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD11_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD11_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD12_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD12_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD13_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD13_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD14_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD14_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD15_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD15_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD16_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD16_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD17_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD17_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD18_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD18_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD19_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD19_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD20_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD20_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD21_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD21_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD22_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD22_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD23_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD23_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD24_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD24_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD25_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD25_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD26_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD26_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD27_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD27_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD28_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD28_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD29_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD29_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD30_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD30_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_TEXTURE_COORD31_EXT");
		lua_pushinteger(lua, GL_OUTPUT_TEXTURE_COORD31_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OUTPUT_FOG_EXT");
		lua_pushinteger(lua, GL_OUTPUT_FOG_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCALAR_EXT");
		lua_pushinteger(lua, GL_SCALAR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VECTOR_EXT");
		lua_pushinteger(lua, GL_VECTOR_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX_EXT");
		lua_pushinteger(lua, GL_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_EXT");
		lua_pushinteger(lua, GL_VARIANT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVARIANT_EXT");
		lua_pushinteger(lua, GL_INVARIANT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOCAL_CONSTANT_EXT");
		lua_pushinteger(lua, GL_LOCAL_CONSTANT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOCAL_EXT");
		lua_pushinteger(lua, GL_LOCAL_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_SHADER_INSTRUCTIONS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_SHADER_VARIANTS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_SHADER_VARIANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_SHADER_INVARIANTS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_SHADER_INVARIANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_SHADER_LOCALS_EXT");
		lua_pushinteger(lua, GL_MAX_VERTEX_SHADER_LOCALS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT");
		lua_pushinteger(lua, GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT");
		lua_pushinteger(lua, GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT");
		lua_pushinteger(lua, GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT");
		lua_pushinteger(lua, GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT");
		lua_pushinteger(lua, GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_INSTRUCTIONS_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_INSTRUCTIONS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_VARIANTS_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_VARIANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_INVARIANTS_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_INVARIANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_LOCAL_CONSTANTS_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_LOCALS_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_LOCALS_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_SHADER_OPTIMIZED_EXT");
		lua_pushinteger(lua, GL_VERTEX_SHADER_OPTIMIZED_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "X_EXT");
		lua_pushinteger(lua, GL_X_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Y_EXT");
		lua_pushinteger(lua, GL_Y_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Z_EXT");
		lua_pushinteger(lua, GL_Z_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "W_EXT");
		lua_pushinteger(lua, GL_W_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEGATIVE_X_EXT");
		lua_pushinteger(lua, GL_NEGATIVE_X_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEGATIVE_Y_EXT");
		lua_pushinteger(lua, GL_NEGATIVE_Y_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEGATIVE_Z_EXT");
		lua_pushinteger(lua, GL_NEGATIVE_Z_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEGATIVE_W_EXT");
		lua_pushinteger(lua, GL_NEGATIVE_W_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ZERO_EXT");
		lua_pushinteger(lua, GL_ZERO_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ONE_EXT");
		lua_pushinteger(lua, GL_ONE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEGATIVE_ONE_EXT");
		lua_pushinteger(lua, GL_NEGATIVE_ONE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMALIZED_RANGE_EXT");
		lua_pushinteger(lua, GL_NORMALIZED_RANGE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FULL_RANGE_EXT");
		lua_pushinteger(lua, GL_FULL_RANGE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_VERTEX_EXT");
		lua_pushinteger(lua, GL_CURRENT_VERTEX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MVP_MATRIX_EXT");
		lua_pushinteger(lua, GL_MVP_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_VALUE_EXT");
		lua_pushinteger(lua, GL_VARIANT_VALUE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_DATATYPE_EXT");
		lua_pushinteger(lua, GL_VARIANT_DATATYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_VARIANT_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_VARIANT_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_ARRAY_EXT");
		lua_pushinteger(lua, GL_VARIANT_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIANT_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_VARIANT_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVARIANT_VALUE_EXT");
		lua_pushinteger(lua, GL_INVARIANT_VALUE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVARIANT_DATATYPE_EXT");
		lua_pushinteger(lua, GL_INVARIANT_DATATYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOCAL_CONSTANT_VALUE_EXT");
		lua_pushinteger(lua, GL_LOCAL_CONSTANT_VALUE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LOCAL_CONSTANT_DATATYPE_EXT");
		lua_pushinteger(lua, GL_LOCAL_CONSTANT_DATATYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXT_vertex_weighting");
		lua_pushinteger(lua, GL_EXT_vertex_weighting);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW0_STACK_DEPTH_EXT");
		lua_pushinteger(lua, GL_MODELVIEW0_STACK_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW0_MATRIX_EXT");
		lua_pushinteger(lua, GL_MODELVIEW0_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW0_EXT");
		lua_pushinteger(lua, GL_MODELVIEW0_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW1_STACK_DEPTH_EXT");
		lua_pushinteger(lua, GL_MODELVIEW1_STACK_DEPTH_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW1_MATRIX_EXT");
		lua_pushinteger(lua, GL_MODELVIEW1_MATRIX_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_WEIGHTING_EXT");
		lua_pushinteger(lua, GL_VERTEX_WEIGHTING_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW1_EXT");
		lua_pushinteger(lua, GL_MODELVIEW1_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_VERTEX_WEIGHT_EXT");
		lua_pushinteger(lua, GL_CURRENT_VERTEX_WEIGHT_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_WEIGHT_ARRAY_EXT");
		lua_pushinteger(lua, GL_VERTEX_WEIGHT_ARRAY_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_WEIGHT_ARRAY_SIZE_EXT");
		lua_pushinteger(lua, GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_WEIGHT_ARRAY_TYPE_EXT");
		lua_pushinteger(lua, GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_WEIGHT_ARRAY_STRIDE_EXT");
		lua_pushinteger(lua, GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_WEIGHT_ARRAY_POINTER_EXT");
		lua_pushinteger(lua, GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXT_x11_sync_object");
//		lua_pushinteger(lua, GL_EXT_x11_sync_object);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SYNC_X11_FENCE_EXT");
//		lua_pushinteger(lua, GL_SYNC_X11_FENCE_EXT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "GREMEDY_frame_terminator");
		lua_pushinteger(lua, GL_GREMEDY_frame_terminator);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREMEDY_string_marker");
		lua_pushinteger(lua, GL_GREMEDY_string_marker);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HP_convolution_border_modes");
		lua_pushinteger(lua, GL_HP_convolution_border_modes);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HP_image_transform");
		lua_pushinteger(lua, GL_HP_image_transform);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HP_occlusion_test");
		lua_pushinteger(lua, GL_HP_occlusion_test);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HP_texture_lighting");
		lua_pushinteger(lua, GL_HP_texture_lighting);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IBM_cull_vertex");
		lua_pushinteger(lua, GL_IBM_cull_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_VERTEX_IBM");
		lua_pushinteger(lua, GL_CULL_VERTEX_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IBM_multimode_draw_arrays");
		lua_pushinteger(lua, GL_IBM_multimode_draw_arrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IBM_rasterpos_clip");
		lua_pushinteger(lua, GL_IBM_rasterpos_clip);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RASTER_POSITION_UNCLIPPED_IBM");
		lua_pushinteger(lua, GL_RASTER_POSITION_UNCLIPPED_IBM);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "IBM_static_data");
//		lua_pushinteger(lua, GL_IBM_static_data);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ALL_STATIC_DATA_IBM");
//		lua_pushinteger(lua, GL_ALL_STATIC_DATA_IBM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STATIC_VERTEX_ARRAY_IBM");
//		lua_pushinteger(lua, GL_STATIC_VERTEX_ARRAY_IBM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "IBM_texture_mirrored_repeat");
//		lua_pushinteger(lua, GL_IBM_texture_mirrored_repeat);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "MIRRORED_REPEAT_IBM");
		lua_pushinteger(lua, GL_MIRRORED_REPEAT_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IBM_vertex_array_lists");
		lua_pushinteger(lua, GL_IBM_vertex_array_lists);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_COLOR_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_INDEX_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_LIST_IBM");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_LIST_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_COLOR_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_INDEX_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INGR_color_clamp");
		lua_pushinteger(lua, GL_INGR_color_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_MIN_CLAMP_INGR");
		lua_pushinteger(lua, GL_RED_MIN_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_MIN_CLAMP_INGR");
		lua_pushinteger(lua, GL_GREEN_MIN_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_MIN_CLAMP_INGR");
		lua_pushinteger(lua, GL_BLUE_MIN_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_MIN_CLAMP_INGR");
		lua_pushinteger(lua, GL_ALPHA_MIN_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RED_MAX_CLAMP_INGR");
		lua_pushinteger(lua, GL_RED_MAX_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GREEN_MAX_CLAMP_INGR");
		lua_pushinteger(lua, GL_GREEN_MAX_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BLUE_MAX_CLAMP_INGR");
		lua_pushinteger(lua, GL_BLUE_MAX_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_MAX_CLAMP_INGR");
		lua_pushinteger(lua, GL_ALPHA_MAX_CLAMP_INGR);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INGR_interlace_read");
		lua_pushinteger(lua, GL_INGR_interlace_read);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLACE_READ_INGR");
		lua_pushinteger(lua, GL_INTERLACE_READ_INGR);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTEL_fragment_shader_ordering");
//		lua_pushinteger(lua, GL_INTEL_fragment_shader_ordering);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTEL_map_texture");
//		lua_pushinteger(lua, GL_INTEL_map_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LAYOUT_DEFAULT_INTEL");
//		lua_pushinteger(lua, GL_LAYOUT_DEFAULT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LAYOUT_LINEAR_INTEL");
//		lua_pushinteger(lua, GL_LAYOUT_LINEAR_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LAYOUT_LINEAR_CPU_CACHED_INTEL");
//		lua_pushinteger(lua, GL_LAYOUT_LINEAR_CPU_CACHED_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_MEMORY_LAYOUT_INTEL");
//		lua_pushinteger(lua, GL_TEXTURE_MEMORY_LAYOUT_INTEL);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "INTEL_parallel_arrays");
		lua_pushinteger(lua, GL_INTEL_parallel_arrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PARALLEL_ARRAYS_INTEL");
		lua_pushinteger(lua, GL_PARALLEL_ARRAYS_INTEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_PARALLEL_POINTERS_INTEL");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_PARALLEL_POINTERS_INTEL");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_PARALLEL_POINTERS_INTEL");
		lua_pushinteger(lua, GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTEL_performance_query");
//		lua_pushinteger(lua, GL_INTEL_performance_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_SINGLE_CONTEXT_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_SINGLE_CONTEXT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_GLOBAL_CONTEXT_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_GLOBAL_CONTEXT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_DONOT_FLUSH_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_DONOT_FLUSH_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_FLUSH_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_FLUSH_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_WAIT_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_WAIT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_EVENT_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_EVENT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DURATION_NORM_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DURATION_NORM_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DURATION_RAW_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DURATION_RAW_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_THROUGHPUT_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_THROUGHPUT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_RAW_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_RAW_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_TIMESTAMP_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_TIMESTAMP_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DATA_UINT32_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DATA_UINT32_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DATA_UINT64_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DATA_UINT64_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DATA_FLOAT_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DATA_FLOAT_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DATA_DOUBLE_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DATA_DOUBLE_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DATA_BOOL32_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DATA_BOOL32_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL");
//		lua_pushinteger(lua, GL_PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTEL_texture_scissor");
//		lua_pushinteger(lua, GL_INTEL_texture_scissor);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_blend_equation_advanced");
//		lua_pushinteger(lua, GL_KHR_blend_equation_advanced);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BLEND_ADVANCED_COHERENT_KHR");
//		lua_pushinteger(lua, GL_BLEND_ADVANCED_COHERENT_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MULTIPLY_KHR");
//		lua_pushinteger(lua, GL_MULTIPLY_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SCREEN_KHR");
//		lua_pushinteger(lua, GL_SCREEN_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "OVERLAY_KHR");
//		lua_pushinteger(lua, GL_OVERLAY_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DARKEN_KHR");
//		lua_pushinteger(lua, GL_DARKEN_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LIGHTEN_KHR");
//		lua_pushinteger(lua, GL_LIGHTEN_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLORDODGE_KHR");
//		lua_pushinteger(lua, GL_COLORDODGE_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLORBURN_KHR");
//		lua_pushinteger(lua, GL_COLORBURN_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HARDLIGHT_KHR");
//		lua_pushinteger(lua, GL_HARDLIGHT_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SOFTLIGHT_KHR");
//		lua_pushinteger(lua, GL_SOFTLIGHT_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DIFFERENCE_KHR");
//		lua_pushinteger(lua, GL_DIFFERENCE_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXCLUSION_KHR");
//		lua_pushinteger(lua, GL_EXCLUSION_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_HUE_KHR");
//		lua_pushinteger(lua, GL_HSL_HUE_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_SATURATION_KHR");
//		lua_pushinteger(lua, GL_HSL_SATURATION_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_COLOR_KHR");
//		lua_pushinteger(lua, GL_HSL_COLOR_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_LUMINOSITY_KHR");
//		lua_pushinteger(lua, GL_HSL_LUMINOSITY_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_blend_equation_advanced_coherent");
//		lua_pushinteger(lua, GL_KHR_blend_equation_advanced_coherent);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_context_flush_control");
//		lua_pushinteger(lua, GL_KHR_context_flush_control);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONTEXT_RELEASE_BEHAVIOR");
//		lua_pushinteger(lua, GL_CONTEXT_RELEASE_BEHAVIOR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONTEXT_RELEASE_BEHAVIOR_FLUSH");
//		lua_pushinteger(lua, GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_debug");
//		lua_pushinteger(lua, GL_KHR_debug);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONTEXT_FLAG_DEBUG_BIT");
//		lua_pushinteger(lua, GL_CONTEXT_FLAG_DEBUG_BIT);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "STACK_OVERFLOW");
		lua_pushinteger(lua, GL_STACK_OVERFLOW);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STACK_UNDERFLOW");
		lua_pushinteger(lua, GL_STACK_UNDERFLOW);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_OUTPUT_SYNCHRONOUS");
//		lua_pushinteger(lua, GL_DEBUG_OUTPUT_SYNCHRONOUS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_NEXT_LOGGED_MESSAGE_LENGTH");
//		lua_pushinteger(lua, GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_CALLBACK_FUNCTION");
//		lua_pushinteger(lua, GL_DEBUG_CALLBACK_FUNCTION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_CALLBACK_USER_PARAM");
//		lua_pushinteger(lua, GL_DEBUG_CALLBACK_USER_PARAM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SOURCE_API");
//		lua_pushinteger(lua, GL_DEBUG_SOURCE_API);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SOURCE_WINDOW_SYSTEM");
//		lua_pushinteger(lua, GL_DEBUG_SOURCE_WINDOW_SYSTEM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SOURCE_SHADER_COMPILER");
//		lua_pushinteger(lua, GL_DEBUG_SOURCE_SHADER_COMPILER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SOURCE_THIRD_PARTY");
//		lua_pushinteger(lua, GL_DEBUG_SOURCE_THIRD_PARTY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SOURCE_APPLICATION");
//		lua_pushinteger(lua, GL_DEBUG_SOURCE_APPLICATION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SOURCE_OTHER");
//		lua_pushinteger(lua, GL_DEBUG_SOURCE_OTHER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_ERROR");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_ERROR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_DEPRECATED_BEHAVIOR");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_UNDEFINED_BEHAVIOR");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_PORTABILITY");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_PORTABILITY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_PERFORMANCE");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_PERFORMANCE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_OTHER");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_OTHER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_MARKER");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_MARKER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_PUSH_GROUP");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_PUSH_GROUP);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_TYPE_POP_GROUP");
//		lua_pushinteger(lua, GL_DEBUG_TYPE_POP_GROUP);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SEVERITY_NOTIFICATION");
//		lua_pushinteger(lua, GL_DEBUG_SEVERITY_NOTIFICATION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_DEBUG_GROUP_STACK_DEPTH");
//		lua_pushinteger(lua, GL_MAX_DEBUG_GROUP_STACK_DEPTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_GROUP_STACK_DEPTH");
//		lua_pushinteger(lua, GL_DEBUG_GROUP_STACK_DEPTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BUFFER");
//		lua_pushinteger(lua, GL_BUFFER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHADER");
//		lua_pushinteger(lua, GL_SHADER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM");
//		lua_pushinteger(lua, GL_PROGRAM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY");
//		lua_pushinteger(lua, GL_QUERY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAM_PIPELINE");
//		lua_pushinteger(lua, GL_PROGRAM_PIPELINE);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLER");
//		lua_pushinteger(lua, GL_SAMPLER);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DISPLAY_LIST");
//		lua_pushinteger(lua, GL_DISPLAY_LIST);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_LABEL_LENGTH");
//		lua_pushinteger(lua, GL_MAX_LABEL_LENGTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_DEBUG_MESSAGE_LENGTH");
//		lua_pushinteger(lua, GL_MAX_DEBUG_MESSAGE_LENGTH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_DEBUG_LOGGED_MESSAGES");
//		lua_pushinteger(lua, GL_MAX_DEBUG_LOGGED_MESSAGES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_LOGGED_MESSAGES");
//		lua_pushinteger(lua, GL_DEBUG_LOGGED_MESSAGES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SEVERITY_HIGH");
//		lua_pushinteger(lua, GL_DEBUG_SEVERITY_HIGH);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SEVERITY_MEDIUM");
//		lua_pushinteger(lua, GL_DEBUG_SEVERITY_MEDIUM);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_SEVERITY_LOW");
//		lua_pushinteger(lua, GL_DEBUG_SEVERITY_LOW);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_OUTPUT");
//		lua_pushinteger(lua, GL_DEBUG_OUTPUT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_robust_buffer_access_behavior");
//		lua_pushinteger(lua, GL_KHR_robust_buffer_access_behavior);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_robustness");
//		lua_pushinteger(lua, GL_KHR_robustness);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONTEXT_LOST");
//		lua_pushinteger(lua, GL_CONTEXT_LOST);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOSE_CONTEXT_ON_RESET");
//		lua_pushinteger(lua, GL_LOSE_CONTEXT_ON_RESET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GUILTY_CONTEXT_RESET");
//		lua_pushinteger(lua, GL_GUILTY_CONTEXT_RESET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INNOCENT_CONTEXT_RESET");
//		lua_pushinteger(lua, GL_INNOCENT_CONTEXT_RESET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNKNOWN_CONTEXT_RESET");
//		lua_pushinteger(lua, GL_UNKNOWN_CONTEXT_RESET);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RESET_NOTIFICATION_STRATEGY");
//		lua_pushinteger(lua, GL_RESET_NOTIFICATION_STRATEGY);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NO_RESET_NOTIFICATION");
//		lua_pushinteger(lua, GL_NO_RESET_NOTIFICATION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONTEXT_ROBUST_ACCESS");
//		lua_pushinteger(lua, GL_CONTEXT_ROBUST_ACCESS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_texture_compression_astc_hdr");
//		lua_pushinteger(lua, GL_KHR_texture_compression_astc_hdr);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_4x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_4x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_5x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_5x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_5x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_5x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_6x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_6x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_6x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_6x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_8x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_8x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_8x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_8x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_8x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_8x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_12x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_12x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_12x12_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_12x12_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KHR_texture_compression_astc_ldr");
//		lua_pushinteger(lua, GL_KHR_texture_compression_astc_ldr);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_4x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_4x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_5x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_5x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_5x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_5x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_6x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_6x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_6x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_6x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_8x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_8x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_8x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_8x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_8x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_8x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_10x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_10x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_12x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_12x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_RGBA_ASTC_12x12_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_RGBA_ASTC_12x12_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR");
//		lua_pushinteger(lua, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KTX_buffer_region");
//		lua_pushinteger(lua, GL_KTX_buffer_region);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KTX_FRONT_REGION");
//		lua_pushinteger(lua, GL_KTX_FRONT_REGION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KTX_BACK_REGION");
//		lua_pushinteger(lua, GL_KTX_BACK_REGION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KTX_Z_REGION");
//		lua_pushinteger(lua, GL_KTX_Z_REGION);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "KTX_STENCIL_REGION");
//		lua_pushinteger(lua, GL_KTX_STENCIL_REGION);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "MESAX_texture_stack");
		lua_pushinteger(lua, GL_MESAX_texture_stack);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D_STACK_MESAX");
		lua_pushinteger(lua, GL_TEXTURE_1D_STACK_MESAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_STACK_MESAX");
		lua_pushinteger(lua, GL_TEXTURE_2D_STACK_MESAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_1D_STACK_MESAX");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_1D_STACK_MESAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_2D_STACK_MESAX");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_2D_STACK_MESAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_1D_STACK_BINDING_MESAX");
		lua_pushinteger(lua, GL_TEXTURE_1D_STACK_BINDING_MESAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_2D_STACK_BINDING_MESAX");
		lua_pushinteger(lua, GL_TEXTURE_2D_STACK_BINDING_MESAX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MESA_pack_invert");
		lua_pushinteger(lua, GL_MESA_pack_invert);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_INVERT_MESA");
		lua_pushinteger(lua, GL_PACK_INVERT_MESA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MESA_resize_buffers");
		lua_pushinteger(lua, GL_MESA_resize_buffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MESA_window_pos");
		lua_pushinteger(lua, GL_MESA_window_pos);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MESA_ycbcr_texture");
		lua_pushinteger(lua, GL_MESA_ycbcr_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_8_8_MESA");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_8_8_MESA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_SHORT_8_8_REV_MESA");
		lua_pushinteger(lua, GL_UNSIGNED_SHORT_8_8_REV_MESA);
		lua_settable(lua, -3);
		lua_pushstring(lua, "YCBCR_MESA");
		lua_pushinteger(lua, GL_YCBCR_MESA);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NVX_conditional_render");
//		lua_pushinteger(lua, GL_NVX_conditional_render);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NVX_gpu_memory_info");
//		lua_pushinteger(lua, GL_NVX_gpu_memory_info);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX");
//		lua_pushinteger(lua, GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX");
//		lua_pushinteger(lua, GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX");
//		lua_pushinteger(lua, GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GPU_MEMORY_INFO_EVICTION_COUNT_NVX");
//		lua_pushinteger(lua, GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GPU_MEMORY_INFO_EVICTED_MEMORY_NVX");
//		lua_pushinteger(lua, GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_bindless_multi_draw_indirect");
//		lua_pushinteger(lua, GL_NV_bindless_multi_draw_indirect);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_bindless_multi_draw_indirect_count");
//		lua_pushinteger(lua, GL_NV_bindless_multi_draw_indirect_count);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_bindless_texture");
//		lua_pushinteger(lua, GL_NV_bindless_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_blend_equation_advanced");
//		lua_pushinteger(lua, GL_NV_blend_equation_advanced);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "XOR_NV");
//		lua_pushinteger(lua, GL_XOR_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RED_NV");
//		lua_pushinteger(lua, GL_RED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GREEN_NV");
//		lua_pushinteger(lua, GL_GREEN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BLUE_NV");
//		lua_pushinteger(lua, GL_BLUE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BLEND_PREMULTIPLIED_SRC_NV");
//		lua_pushinteger(lua, GL_BLEND_PREMULTIPLIED_SRC_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BLEND_OVERLAP_NV");
//		lua_pushinteger(lua, GL_BLEND_OVERLAP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNCORRELATED_NV");
//		lua_pushinteger(lua, GL_UNCORRELATED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DISJOINT_NV");
//		lua_pushinteger(lua, GL_DISJOINT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONJOINT_NV");
//		lua_pushinteger(lua, GL_CONJOINT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BLEND_ADVANCED_COHERENT_NV");
//		lua_pushinteger(lua, GL_BLEND_ADVANCED_COHERENT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRC_NV");
//		lua_pushinteger(lua, GL_SRC_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DST_NV");
//		lua_pushinteger(lua, GL_DST_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRC_OVER_NV");
//		lua_pushinteger(lua, GL_SRC_OVER_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DST_OVER_NV");
//		lua_pushinteger(lua, GL_DST_OVER_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRC_IN_NV");
//		lua_pushinteger(lua, GL_SRC_IN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DST_IN_NV");
//		lua_pushinteger(lua, GL_DST_IN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRC_OUT_NV");
//		lua_pushinteger(lua, GL_SRC_OUT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DST_OUT_NV");
//		lua_pushinteger(lua, GL_DST_OUT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SRC_ATOP_NV");
//		lua_pushinteger(lua, GL_SRC_ATOP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DST_ATOP_NV");
//		lua_pushinteger(lua, GL_DST_ATOP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PLUS_NV");
//		lua_pushinteger(lua, GL_PLUS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PLUS_DARKER_NV");
//		lua_pushinteger(lua, GL_PLUS_DARKER_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MULTIPLY_NV");
//		lua_pushinteger(lua, GL_MULTIPLY_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SCREEN_NV");
//		lua_pushinteger(lua, GL_SCREEN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "OVERLAY_NV");
//		lua_pushinteger(lua, GL_OVERLAY_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DARKEN_NV");
//		lua_pushinteger(lua, GL_DARKEN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LIGHTEN_NV");
//		lua_pushinteger(lua, GL_LIGHTEN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLORDODGE_NV");
//		lua_pushinteger(lua, GL_COLORDODGE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COLORBURN_NV");
//		lua_pushinteger(lua, GL_COLORBURN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HARDLIGHT_NV");
//		lua_pushinteger(lua, GL_HARDLIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SOFTLIGHT_NV");
//		lua_pushinteger(lua, GL_SOFTLIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DIFFERENCE_NV");
//		lua_pushinteger(lua, GL_DIFFERENCE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MINUS_NV");
//		lua_pushinteger(lua, GL_MINUS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXCLUSION_NV");
//		lua_pushinteger(lua, GL_EXCLUSION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONTRAST_NV");
//		lua_pushinteger(lua, GL_CONTRAST_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INVERT_RGB_NV");
//		lua_pushinteger(lua, GL_INVERT_RGB_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LINEARDODGE_NV");
//		lua_pushinteger(lua, GL_LINEARDODGE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LINEARBURN_NV");
//		lua_pushinteger(lua, GL_LINEARBURN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VIVIDLIGHT_NV");
//		lua_pushinteger(lua, GL_VIVIDLIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LINEARLIGHT_NV");
//		lua_pushinteger(lua, GL_LINEARLIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PINLIGHT_NV");
//		lua_pushinteger(lua, GL_PINLIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HARDMIX_NV");
//		lua_pushinteger(lua, GL_HARDMIX_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_HUE_NV");
//		lua_pushinteger(lua, GL_HSL_HUE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_SATURATION_NV");
//		lua_pushinteger(lua, GL_HSL_SATURATION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_COLOR_NV");
//		lua_pushinteger(lua, GL_HSL_COLOR_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HSL_LUMINOSITY_NV");
//		lua_pushinteger(lua, GL_HSL_LUMINOSITY_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PLUS_CLAMPED_NV");
//		lua_pushinteger(lua, GL_PLUS_CLAMPED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PLUS_CLAMPED_ALPHA_NV");
//		lua_pushinteger(lua, GL_PLUS_CLAMPED_ALPHA_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MINUS_CLAMPED_NV");
//		lua_pushinteger(lua, GL_MINUS_CLAMPED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INVERT_OVG_NV");
//		lua_pushinteger(lua, GL_INVERT_OVG_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_blend_equation_advanced_coherent");
//		lua_pushinteger(lua, GL_NV_blend_equation_advanced_coherent);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_blend_square");
		lua_pushinteger(lua, GL_NV_blend_square);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_compute_program5");
//		lua_pushinteger(lua, GL_NV_compute_program5);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_PROGRAM_NV");
//		lua_pushinteger(lua, GL_COMPUTE_PROGRAM_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COMPUTE_PROGRAM_PARAMETER_BUFFER_NV");
//		lua_pushinteger(lua, GL_COMPUTE_PROGRAM_PARAMETER_BUFFER_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_conditional_render");
		lua_pushinteger(lua, GL_NV_conditional_render);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_WAIT_NV");
		lua_pushinteger(lua, GL_QUERY_WAIT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_NO_WAIT_NV");
		lua_pushinteger(lua, GL_QUERY_NO_WAIT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_BY_REGION_WAIT_NV");
		lua_pushinteger(lua, GL_QUERY_BY_REGION_WAIT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUERY_BY_REGION_NO_WAIT_NV");
		lua_pushinteger(lua, GL_QUERY_BY_REGION_NO_WAIT_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_conservative_raster");
//		lua_pushinteger(lua, GL_NV_conservative_raster);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONSERVATIVE_RASTERIZATION_NV");
//		lua_pushinteger(lua, GL_CONSERVATIVE_RASTERIZATION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SUBPIXEL_PRECISION_BIAS_X_BITS_NV");
//		lua_pushinteger(lua, GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SUBPIXEL_PRECISION_BIAS_Y_BITS_NV");
//		lua_pushinteger(lua, GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV");
//		lua_pushinteger(lua, GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_copy_depth_to_color");
		lua_pushinteger(lua, GL_NV_copy_depth_to_color);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_STENCIL_TO_RGBA_NV");
		lua_pushinteger(lua, GL_DEPTH_STENCIL_TO_RGBA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_STENCIL_TO_BGRA_NV");
		lua_pushinteger(lua, GL_DEPTH_STENCIL_TO_BGRA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_copy_image");
		lua_pushinteger(lua, GL_NV_copy_image);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_deep_texture3D");
//		lua_pushinteger(lua, GL_NV_deep_texture3D);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT_NV");
//		lua_pushinteger(lua, GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_DEEP_3D_TEXTURE_DEPTH_NV");
//		lua_pushinteger(lua, GL_MAX_DEEP_3D_TEXTURE_DEPTH_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_depth_buffer_float");
		lua_pushinteger(lua, GL_NV_depth_buffer_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT32F_NV");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT32F_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH32F_STENCIL8_NV");
		lua_pushinteger(lua, GL_DEPTH32F_STENCIL8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_32_UNSIGNED_INT_24_8_REV_NV");
		lua_pushinteger(lua, GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_BUFFER_FLOAT_MODE_NV");
		lua_pushinteger(lua, GL_DEPTH_BUFFER_FLOAT_MODE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_depth_clamp");
		lua_pushinteger(lua, GL_NV_depth_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_CLAMP_NV");
		lua_pushinteger(lua, GL_DEPTH_CLAMP_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_depth_range_unclamped");
//		lua_pushinteger(lua, GL_NV_depth_range_unclamped);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLE_COUNT_BITS_NV");
//		lua_pushinteger(lua, GL_SAMPLE_COUNT_BITS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CURRENT_SAMPLE_COUNT_QUERY_NV");
//		lua_pushinteger(lua, GL_CURRENT_SAMPLE_COUNT_QUERY_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_RESULT_NV");
//		lua_pushinteger(lua, GL_QUERY_RESULT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUERY_RESULT_AVAILABLE_NV");
//		lua_pushinteger(lua, GL_QUERY_RESULT_AVAILABLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLE_COUNT_NV");
//		lua_pushinteger(lua, GL_SAMPLE_COUNT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_draw_texture");
//		lua_pushinteger(lua, GL_NV_draw_texture);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_evaluators");
		lua_pushinteger(lua, GL_NV_evaluators);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_2D_NV");
		lua_pushinteger(lua, GL_EVAL_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_TRIANGULAR_2D_NV");
		lua_pushinteger(lua, GL_EVAL_TRIANGULAR_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_TESSELLATION_NV");
		lua_pushinteger(lua, GL_MAP_TESSELLATION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_ATTRIB_U_ORDER_NV");
		lua_pushinteger(lua, GL_MAP_ATTRIB_U_ORDER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP_ATTRIB_V_ORDER_NV");
		lua_pushinteger(lua, GL_MAP_ATTRIB_V_ORDER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_FRACTIONAL_TESSELLATION_NV");
		lua_pushinteger(lua, GL_EVAL_FRACTIONAL_TESSELLATION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB0_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB0_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB1_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB1_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB2_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB3_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB4_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB5_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB5_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB6_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB6_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB7_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB7_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB8_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB9_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB9_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB10_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB10_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB11_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB11_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB12_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB12_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB13_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB13_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB14_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB14_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EVAL_VERTEX_ATTRIB15_NV");
		lua_pushinteger(lua, GL_EVAL_VERTEX_ATTRIB15_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_MAP_TESSELLATION_NV");
		lua_pushinteger(lua, GL_MAX_MAP_TESSELLATION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_RATIONAL_EVAL_ORDER_NV");
		lua_pushinteger(lua, GL_MAX_RATIONAL_EVAL_ORDER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_explicit_multisample");
		lua_pushinteger(lua, GL_NV_explicit_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_POSITION_NV");
		lua_pushinteger(lua, GL_SAMPLE_POSITION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_NV");
		lua_pushinteger(lua, GL_SAMPLE_MASK_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_VALUE_NV");
		lua_pushinteger(lua, GL_SAMPLE_MASK_VALUE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_RENDERBUFFER_NV");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_RENDERBUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV");
		lua_pushinteger(lua, GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RENDERBUFFER_NV");
		lua_pushinteger(lua, GL_TEXTURE_RENDERBUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLER_RENDERBUFFER_NV");
		lua_pushinteger(lua, GL_SAMPLER_RENDERBUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT_SAMPLER_RENDERBUFFER_NV");
		lua_pushinteger(lua, GL_INT_SAMPLER_RENDERBUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SAMPLE_MASK_WORDS_NV");
		lua_pushinteger(lua, GL_MAX_SAMPLE_MASK_WORDS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_fence");
		lua_pushinteger(lua, GL_NV_fence);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALL_COMPLETED_NV");
		lua_pushinteger(lua, GL_ALL_COMPLETED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FENCE_STATUS_NV");
		lua_pushinteger(lua, GL_FENCE_STATUS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FENCE_CONDITION_NV");
		lua_pushinteger(lua, GL_FENCE_CONDITION_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_fill_rectangle");
//		lua_pushinteger(lua, GL_NV_fill_rectangle);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FILL_RECTANGLE_NV");
//		lua_pushinteger(lua, GL_FILL_RECTANGLE_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_float_buffer");
		lua_pushinteger(lua, GL_NV_float_buffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_R_NV");
		lua_pushinteger(lua, GL_FLOAT_R_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RG_NV");
		lua_pushinteger(lua, GL_FLOAT_RG_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGB_NV");
		lua_pushinteger(lua, GL_FLOAT_RGB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGBA_NV");
		lua_pushinteger(lua, GL_FLOAT_RGBA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_R16_NV");
		lua_pushinteger(lua, GL_FLOAT_R16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_R32_NV");
		lua_pushinteger(lua, GL_FLOAT_R32_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RG16_NV");
		lua_pushinteger(lua, GL_FLOAT_RG16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RG32_NV");
		lua_pushinteger(lua, GL_FLOAT_RG32_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGB16_NV");
		lua_pushinteger(lua, GL_FLOAT_RGB16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGB32_NV");
		lua_pushinteger(lua, GL_FLOAT_RGB32_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGBA16_NV");
		lua_pushinteger(lua, GL_FLOAT_RGBA16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGBA32_NV");
		lua_pushinteger(lua, GL_FLOAT_RGBA32_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_FLOAT_COMPONENTS_NV");
		lua_pushinteger(lua, GL_TEXTURE_FLOAT_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_CLEAR_COLOR_VALUE_NV");
		lua_pushinteger(lua, GL_FLOAT_CLEAR_COLOR_VALUE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT_RGBA_MODE_NV");
		lua_pushinteger(lua, GL_FLOAT_RGBA_MODE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_fog_distance");
		lua_pushinteger(lua, GL_NV_fog_distance);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_DISTANCE_MODE_NV");
		lua_pushinteger(lua, GL_FOG_DISTANCE_MODE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_RADIAL_NV");
		lua_pushinteger(lua, GL_EYE_RADIAL_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_PLANE_ABSOLUTE_NV");
		lua_pushinteger(lua, GL_EYE_PLANE_ABSOLUTE_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_fragment_coverage_to_color");
//		lua_pushinteger(lua, GL_NV_fragment_coverage_to_color);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_COVERAGE_TO_COLOR_NV");
//		lua_pushinteger(lua, GL_FRAGMENT_COVERAGE_TO_COLOR_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_COVERAGE_COLOR_NV");
//		lua_pushinteger(lua, GL_FRAGMENT_COVERAGE_COLOR_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_fragment_program");
		lua_pushinteger(lua, GL_NV_fragment_program);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_PROGRAM_NV");
		lua_pushinteger(lua, GL_FRAGMENT_PROGRAM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_COORDS_NV");
		lua_pushinteger(lua, GL_MAX_TEXTURE_COORDS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TEXTURE_IMAGE_UNITS_NV");
		lua_pushinteger(lua, GL_MAX_TEXTURE_IMAGE_UNITS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_PROGRAM_BINDING_NV");
		lua_pushinteger(lua, GL_FRAGMENT_PROGRAM_BINDING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ERROR_STRING_NV");
		lua_pushinteger(lua, GL_PROGRAM_ERROR_STRING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_fragment_program2");
		lua_pushinteger(lua, GL_NV_fragment_program2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_EXEC_INSTRUCTIONS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_CALL_DEPTH_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_CALL_DEPTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_IF_DEPTH_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_IF_DEPTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_LOOP_DEPTH_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_LOOP_DEPTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_LOOP_COUNT_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_LOOP_COUNT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_fragment_program4");
		lua_pushinteger(lua, GL_NV_fragment_program4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_fragment_program_option");
		lua_pushinteger(lua, GL_NV_fragment_program_option);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_fragment_shader_interlock");
//		lua_pushinteger(lua, GL_NV_fragment_shader_interlock);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_framebuffer_mixed_samples");
//		lua_pushinteger(lua, GL_NV_framebuffer_mixed_samples);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SAMPLES_NV");
		lua_pushinteger(lua, GL_COLOR_SAMPLES_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "RASTER_MULTISAMPLE_EXT");
//		lua_pushinteger(lua, GL_RASTER_MULTISAMPLE_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RASTER_SAMPLES_EXT");
//		lua_pushinteger(lua, GL_RASTER_SAMPLES_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_RASTER_SAMPLES_EXT");
//		lua_pushinteger(lua, GL_MAX_RASTER_SAMPLES_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RASTER_FIXED_SAMPLE_LOCATIONS_EXT");
//		lua_pushinteger(lua, GL_RASTER_FIXED_SAMPLE_LOCATIONS_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MULTISAMPLE_RASTERIZATION_ALLOWED_EXT");
//		lua_pushinteger(lua, GL_MULTISAMPLE_RASTERIZATION_ALLOWED_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EFFECTIVE_RASTER_SAMPLES_EXT");
//		lua_pushinteger(lua, GL_EFFECTIVE_RASTER_SAMPLES_EXT);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEPTH_SAMPLES_NV");
//		lua_pushinteger(lua, GL_DEPTH_SAMPLES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STENCIL_SAMPLES_NV");
//		lua_pushinteger(lua, GL_STENCIL_SAMPLES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIXED_DEPTH_SAMPLES_SUPPORTED_NV");
//		lua_pushinteger(lua, GL_MIXED_DEPTH_SAMPLES_SUPPORTED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIXED_STENCIL_SAMPLES_SUPPORTED_NV");
//		lua_pushinteger(lua, GL_MIXED_STENCIL_SAMPLES_SUPPORTED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COVERAGE_MODULATION_TABLE_NV");
//		lua_pushinteger(lua, GL_COVERAGE_MODULATION_TABLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COVERAGE_MODULATION_NV");
//		lua_pushinteger(lua, GL_COVERAGE_MODULATION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COVERAGE_MODULATION_TABLE_SIZE_NV");
//		lua_pushinteger(lua, GL_COVERAGE_MODULATION_TABLE_SIZE_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_framebuffer_multisample_coverage");
		lua_pushinteger(lua, GL_NV_framebuffer_multisample_coverage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_COVERAGE_SAMPLES_NV");
		lua_pushinteger(lua, GL_RENDERBUFFER_COVERAGE_SAMPLES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RENDERBUFFER_COLOR_SAMPLES_NV");
		lua_pushinteger(lua, GL_RENDERBUFFER_COLOR_SAMPLES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_MULTISAMPLE_COVERAGE_MODES_NV");
		lua_pushinteger(lua, GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_COVERAGE_MODES_NV");
		lua_pushinteger(lua, GL_MULTISAMPLE_COVERAGE_MODES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_geometry_program4");
		lua_pushinteger(lua, GL_NV_geometry_program4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_PROGRAM_NV");
		lua_pushinteger(lua, GL_GEOMETRY_PROGRAM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_OUTPUT_VERTICES_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_OUTPUT_VERTICES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_geometry_shader4");
		lua_pushinteger(lua, GL_NV_geometry_shader4);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_geometry_shader_passthrough");
//		lua_pushinteger(lua, GL_NV_geometry_shader_passthrough);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_gpu_program4");
		lua_pushinteger(lua, GL_NV_gpu_program4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_PROGRAM_TEXEL_OFFSET_NV");
		lua_pushinteger(lua, GL_MIN_PROGRAM_TEXEL_OFFSET_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_TEXEL_OFFSET_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_TEXEL_OFFSET_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ATTRIB_COMPONENTS_NV");
		lua_pushinteger(lua, GL_PROGRAM_ATTRIB_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_RESULT_COMPONENTS_NV");
		lua_pushinteger(lua, GL_PROGRAM_RESULT_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_ATTRIB_COMPONENTS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_RESULT_COMPONENTS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_RESULT_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_GENERIC_ATTRIBS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_GENERIC_RESULTS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_GENERIC_RESULTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_gpu_program5");
		lua_pushinteger(lua, GL_NV_gpu_program5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV");
		lua_pushinteger(lua, GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MIN_FRAGMENT_INTERPOLATION_OFFSET_NV");
		lua_pushinteger(lua, GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_FRAGMENT_INTERPOLATION_OFFSET_NV");
		lua_pushinteger(lua, GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV");
		lua_pushinteger(lua, GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV");
//		lua_pushinteger(lua, GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV");
//		lua_pushinteger(lua, GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_gpu_program5_mem_extended");
//		lua_pushinteger(lua, GL_NV_gpu_program5_mem_extended);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_gpu_program_fp64");
//		lua_pushinteger(lua, GL_NV_gpu_program_fp64);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_gpu_shader5");
		lua_pushinteger(lua, GL_NV_gpu_shader5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT64_NV");
		lua_pushinteger(lua, GL_INT64_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT64_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT64_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT8_NV");
		lua_pushinteger(lua, GL_INT8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT8_VEC2_NV");
		lua_pushinteger(lua, GL_INT8_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT8_VEC3_NV");
		lua_pushinteger(lua, GL_INT8_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT8_VEC4_NV");
		lua_pushinteger(lua, GL_INT8_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT16_NV");
		lua_pushinteger(lua, GL_INT16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT16_VEC2_NV");
		lua_pushinteger(lua, GL_INT16_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT16_VEC3_NV");
		lua_pushinteger(lua, GL_INT16_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT16_VEC4_NV");
		lua_pushinteger(lua, GL_INT16_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT64_VEC2_NV");
		lua_pushinteger(lua, GL_INT64_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT64_VEC3_NV");
		lua_pushinteger(lua, GL_INT64_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT64_VEC4_NV");
		lua_pushinteger(lua, GL_INT64_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT8_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT8_VEC2_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT8_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT8_VEC3_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT8_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT8_VEC4_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT8_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT16_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT16_VEC2_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT16_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT16_VEC3_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT16_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT16_VEC4_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT16_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT64_VEC2_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT64_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT64_VEC3_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT64_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT64_VEC4_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT64_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT16_NV");
		lua_pushinteger(lua, GL_FLOAT16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT16_VEC2_NV");
		lua_pushinteger(lua, GL_FLOAT16_VEC2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT16_VEC3_NV");
		lua_pushinteger(lua, GL_FLOAT16_VEC3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FLOAT16_VEC4_NV");
		lua_pushinteger(lua, GL_FLOAT16_VEC4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_half_float");
		lua_pushinteger(lua, GL_NV_half_float);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_FLOAT_NV");
		lua_pushinteger(lua, GL_HALF_FLOAT_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_internalformat_sample_query");
//		lua_pushinteger(lua, GL_NV_internalformat_sample_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MULTISAMPLES_NV");
//		lua_pushinteger(lua, GL_MULTISAMPLES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SUPERSAMPLE_SCALE_X_NV");
//		lua_pushinteger(lua, GL_SUPERSAMPLE_SCALE_X_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SUPERSAMPLE_SCALE_Y_NV");
//		lua_pushinteger(lua, GL_SUPERSAMPLE_SCALE_Y_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONFORMANT_NV");
//		lua_pushinteger(lua, GL_CONFORMANT_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_light_max_exponent");
		lua_pushinteger(lua, GL_NV_light_max_exponent);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SHININESS_NV");
		lua_pushinteger(lua, GL_MAX_SHININESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SPOT_EXPONENT_NV");
		lua_pushinteger(lua, GL_MAX_SPOT_EXPONENT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_multisample_coverage");
		lua_pushinteger(lua, GL_NV_multisample_coverage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SAMPLES_NV");
		lua_pushinteger(lua, GL_COLOR_SAMPLES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_multisample_filter_hint");
		lua_pushinteger(lua, GL_NV_multisample_filter_hint);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_FILTER_HINT_NV");
		lua_pushinteger(lua, GL_MULTISAMPLE_FILTER_HINT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_occlusion_query");
		lua_pushinteger(lua, GL_NV_occlusion_query);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_COUNTER_BITS_NV");
		lua_pushinteger(lua, GL_PIXEL_COUNTER_BITS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_OCCLUSION_QUERY_ID_NV");
		lua_pushinteger(lua, GL_CURRENT_OCCLUSION_QUERY_ID_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_COUNT_NV");
		lua_pushinteger(lua, GL_PIXEL_COUNT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PIXEL_COUNT_AVAILABLE_NV");
		lua_pushinteger(lua, GL_PIXEL_COUNT_AVAILABLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_packed_depth_stencil");
		lua_pushinteger(lua, GL_NV_packed_depth_stencil);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_STENCIL_NV");
		lua_pushinteger(lua, GL_DEPTH_STENCIL_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_24_8_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_24_8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_parameter_buffer_object");
		lua_pushinteger(lua, GL_NV_parameter_buffer_object);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_PARAMETER_BUFFER_NV");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV");
		lua_pushinteger(lua, GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV");
		lua_pushinteger(lua, GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_parameter_buffer_object2");
		lua_pushinteger(lua, GL_NV_parameter_buffer_object2);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_path_rendering");
//		lua_pushinteger(lua, GL_NV_path_rendering);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CLOSE_PATH_NV");
//		lua_pushinteger(lua, GL_CLOSE_PATH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BOLD_BIT_NV");
//		lua_pushinteger(lua, GL_BOLD_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_WIDTH_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_WIDTH_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_HEIGHT_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_HEIGHT_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ITALIC_BIT_NV");
//		lua_pushinteger(lua, GL_ITALIC_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MOVE_TO_NV");
//		lua_pushinteger(lua, GL_MOVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_MOVE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_MOVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_HORIZONTAL_BEARING_X_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_HORIZONTAL_BEARING_X_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LINE_TO_NV");
//		lua_pushinteger(lua, GL_LINE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_LINE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_LINE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "HORIZONTAL_LINE_TO_NV");
//		lua_pushinteger(lua, GL_HORIZONTAL_LINE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_HORIZONTAL_LINE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_HORIZONTAL_LINE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_HORIZONTAL_BEARING_Y_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_HORIZONTAL_BEARING_Y_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "VERTICAL_LINE_TO_NV");
//		lua_pushinteger(lua, GL_VERTICAL_LINE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_VERTICAL_LINE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_VERTICAL_LINE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "QUADRATIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_QUADRATIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_QUADRATIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_QUADRATIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CUBIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_CUBIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_CUBIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_CUBIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SMOOTH_QUADRATIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_SMOOTH_QUADRATIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SMOOTH_CUBIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_SMOOTH_CUBIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SMALL_CCW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_SMALL_CCW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_SMALL_CCW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_SMALL_CCW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SMALL_CW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_SMALL_CW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_SMALL_CW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_SMALL_CW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LARGE_CCW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_LARGE_CCW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_LARGE_CCW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_LARGE_CCW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LARGE_CW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_LARGE_CW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_LARGE_CW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_LARGE_CW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_CONIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_CONIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_CONIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_VERTICAL_BEARING_X_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_VERTICAL_BEARING_X_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_VERTICAL_BEARING_Y_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_VERTICAL_BEARING_Y_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ROUNDED_RECT_NV");
//		lua_pushinteger(lua, GL_ROUNDED_RECT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_ROUNDED_RECT_NV");
//		lua_pushinteger(lua, GL_RELATIVE_ROUNDED_RECT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ROUNDED_RECT2_NV");
//		lua_pushinteger(lua, GL_ROUNDED_RECT2_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_ROUNDED_RECT2_NV");
//		lua_pushinteger(lua, GL_RELATIVE_ROUNDED_RECT2_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ROUNDED_RECT4_NV");
//		lua_pushinteger(lua, GL_ROUNDED_RECT4_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_ROUNDED_RECT4_NV");
//		lua_pushinteger(lua, GL_RELATIVE_ROUNDED_RECT4_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ROUNDED_RECT8_NV");
//		lua_pushinteger(lua, GL_ROUNDED_RECT8_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_ROUNDED_RECT8_NV");
//		lua_pushinteger(lua, GL_RELATIVE_ROUNDED_RECT8_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RESTART_PATH_NV");
//		lua_pushinteger(lua, GL_RESTART_PATH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DUP_FIRST_CUBIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_DUP_FIRST_CUBIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DUP_LAST_CUBIC_CURVE_TO_NV");
//		lua_pushinteger(lua, GL_DUP_LAST_CUBIC_CURVE_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RECT_NV");
//		lua_pushinteger(lua, GL_RECT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_RECT_NV");
//		lua_pushinteger(lua, GL_RELATIVE_RECT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CIRCULAR_CCW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_CIRCULAR_CCW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CIRCULAR_CW_ARC_TO_NV");
//		lua_pushinteger(lua, GL_CIRCULAR_CW_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CIRCULAR_TANGENT_ARC_TO_NV");
//		lua_pushinteger(lua, GL_CIRCULAR_TANGENT_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ARC_TO_NV");
//		lua_pushinteger(lua, GL_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RELATIVE_ARC_TO_NV");
//		lua_pushinteger(lua, GL_RELATIVE_ARC_TO_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "GLYPH_HAS_KERNING_BIT_NV");
//		lua_pushinteger(lua, GL_GLYPH_HAS_KERNING_BIT_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR_NV");
		lua_pushinteger(lua, GL_PRIMARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_NV");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR");
		lua_pushinteger(lua, GL_PRIMARY_COLOR);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FORMAT_SVG_NV");
//		lua_pushinteger(lua, GL_PATH_FORMAT_SVG_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FORMAT_PS_NV");
//		lua_pushinteger(lua, GL_PATH_FORMAT_PS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STANDARD_FONT_NAME_NV");
//		lua_pushinteger(lua, GL_STANDARD_FONT_NAME_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SYSTEM_FONT_NAME_NV");
//		lua_pushinteger(lua, GL_SYSTEM_FONT_NAME_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FILE_NAME_NV");
//		lua_pushinteger(lua, GL_FILE_NAME_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STROKE_WIDTH_NV");
//		lua_pushinteger(lua, GL_PATH_STROKE_WIDTH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_END_CAPS_NV");
//		lua_pushinteger(lua, GL_PATH_END_CAPS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_INITIAL_END_CAP_NV");
//		lua_pushinteger(lua, GL_PATH_INITIAL_END_CAP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_TERMINAL_END_CAP_NV");
//		lua_pushinteger(lua, GL_PATH_TERMINAL_END_CAP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_JOIN_STYLE_NV");
//		lua_pushinteger(lua, GL_PATH_JOIN_STYLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_MITER_LIMIT_NV");
//		lua_pushinteger(lua, GL_PATH_MITER_LIMIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_DASH_CAPS_NV");
//		lua_pushinteger(lua, GL_PATH_DASH_CAPS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_INITIAL_DASH_CAP_NV");
//		lua_pushinteger(lua, GL_PATH_INITIAL_DASH_CAP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_TERMINAL_DASH_CAP_NV");
//		lua_pushinteger(lua, GL_PATH_TERMINAL_DASH_CAP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_DASH_OFFSET_NV");
//		lua_pushinteger(lua, GL_PATH_DASH_OFFSET_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_CLIENT_LENGTH_NV");
//		lua_pushinteger(lua, GL_PATH_CLIENT_LENGTH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FILL_MODE_NV");
//		lua_pushinteger(lua, GL_PATH_FILL_MODE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FILL_MASK_NV");
//		lua_pushinteger(lua, GL_PATH_FILL_MASK_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FILL_COVER_MODE_NV");
//		lua_pushinteger(lua, GL_PATH_FILL_COVER_MODE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STROKE_COVER_MODE_NV");
//		lua_pushinteger(lua, GL_PATH_STROKE_COVER_MODE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STROKE_MASK_NV");
//		lua_pushinteger(lua, GL_PATH_STROKE_MASK_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STROKE_BOUND_NV");
//		lua_pushinteger(lua, GL_PATH_STROKE_BOUND_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COUNT_UP_NV");
//		lua_pushinteger(lua, GL_COUNT_UP_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "COUNT_DOWN_NV");
//		lua_pushinteger(lua, GL_COUNT_DOWN_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_OBJECT_BOUNDING_BOX_NV");
//		lua_pushinteger(lua, GL_PATH_OBJECT_BOUNDING_BOX_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CONVEX_HULL_NV");
//		lua_pushinteger(lua, GL_CONVEX_HULL_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BOUNDING_BOX_NV");
//		lua_pushinteger(lua, GL_BOUNDING_BOX_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSLATE_X_NV");
//		lua_pushinteger(lua, GL_TRANSLATE_X_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSLATE_Y_NV");
//		lua_pushinteger(lua, GL_TRANSLATE_Y_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSLATE_2D_NV");
//		lua_pushinteger(lua, GL_TRANSLATE_2D_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSLATE_3D_NV");
//		lua_pushinteger(lua, GL_TRANSLATE_3D_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AFFINE_2D_NV");
//		lua_pushinteger(lua, GL_AFFINE_2D_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "AFFINE_3D_NV");
//		lua_pushinteger(lua, GL_AFFINE_3D_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSPOSE_AFFINE_2D_NV");
//		lua_pushinteger(lua, GL_TRANSPOSE_AFFINE_2D_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRANSPOSE_AFFINE_3D_NV");
//		lua_pushinteger(lua, GL_TRANSPOSE_AFFINE_3D_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UTF8_NV");
//		lua_pushinteger(lua, GL_UTF8_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UTF16_NV");
//		lua_pushinteger(lua, GL_UTF16_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BOUNDING_BOX_OF_BOUNDING_BOXES_NV");
//		lua_pushinteger(lua, GL_BOUNDING_BOX_OF_BOUNDING_BOXES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_COMMAND_COUNT_NV");
//		lua_pushinteger(lua, GL_PATH_COMMAND_COUNT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_COORD_COUNT_NV");
//		lua_pushinteger(lua, GL_PATH_COORD_COUNT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_DASH_ARRAY_COUNT_NV");
//		lua_pushinteger(lua, GL_PATH_DASH_ARRAY_COUNT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_COMPUTED_LENGTH_NV");
//		lua_pushinteger(lua, GL_PATH_COMPUTED_LENGTH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FILL_BOUNDING_BOX_NV");
//		lua_pushinteger(lua, GL_PATH_FILL_BOUNDING_BOX_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STROKE_BOUNDING_BOX_NV");
//		lua_pushinteger(lua, GL_PATH_STROKE_BOUNDING_BOX_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SQUARE_NV");
//		lua_pushinteger(lua, GL_SQUARE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ROUND_NV");
//		lua_pushinteger(lua, GL_ROUND_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRIANGULAR_NV");
//		lua_pushinteger(lua, GL_TRIANGULAR_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "BEVEL_NV");
//		lua_pushinteger(lua, GL_BEVEL_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MITER_REVERT_NV");
//		lua_pushinteger(lua, GL_MITER_REVERT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MITER_TRUNCATE_NV");
//		lua_pushinteger(lua, GL_MITER_TRUNCATE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SKIP_MISSING_GLYPH_NV");
//		lua_pushinteger(lua, GL_SKIP_MISSING_GLYPH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "USE_MISSING_GLYPH_NV");
//		lua_pushinteger(lua, GL_USE_MISSING_GLYPH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_ERROR_POSITION_NV");
//		lua_pushinteger(lua, GL_PATH_ERROR_POSITION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_FOG_GEN_MODE_NV");
//		lua_pushinteger(lua, GL_PATH_FOG_GEN_MODE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ACCUM_ADJACENT_PAIRS_NV");
//		lua_pushinteger(lua, GL_ACCUM_ADJACENT_PAIRS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ADJACENT_PAIRS_NV");
//		lua_pushinteger(lua, GL_ADJACENT_PAIRS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FIRST_TO_REST_NV");
//		lua_pushinteger(lua, GL_FIRST_TO_REST_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_GEN_MODE_NV");
//		lua_pushinteger(lua, GL_PATH_GEN_MODE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_GEN_COEFF_NV");
//		lua_pushinteger(lua, GL_PATH_GEN_COEFF_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_GEN_COLOR_FORMAT_NV");
//		lua_pushinteger(lua, GL_PATH_GEN_COLOR_FORMAT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_GEN_COMPONENTS_NV");
//		lua_pushinteger(lua, GL_PATH_GEN_COMPONENTS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_DASH_OFFSET_RESET_NV");
//		lua_pushinteger(lua, GL_PATH_DASH_OFFSET_RESET_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MOVE_TO_RESETS_NV");
//		lua_pushinteger(lua, GL_MOVE_TO_RESETS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MOVE_TO_CONTINUES_NV");
//		lua_pushinteger(lua, GL_MOVE_TO_CONTINUES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STENCIL_FUNC_NV");
//		lua_pushinteger(lua, GL_PATH_STENCIL_FUNC_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STENCIL_REF_NV");
//		lua_pushinteger(lua, GL_PATH_STENCIL_REF_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STENCIL_VALUE_MASK_NV");
//		lua_pushinteger(lua, GL_PATH_STENCIL_VALUE_MASK_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV");
//		lua_pushinteger(lua, GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_STENCIL_DEPTH_OFFSET_UNITS_NV");
//		lua_pushinteger(lua, GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PATH_COVER_DEPTH_FUNC_NV");
//		lua_pushinteger(lua, GL_PATH_COVER_DEPTH_FUNC_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_GLYPHS_AVAILABLE_NV");
//		lua_pushinteger(lua, GL_FONT_GLYPHS_AVAILABLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_TARGET_UNAVAILABLE_NV");
//		lua_pushinteger(lua, GL_FONT_TARGET_UNAVAILABLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_UNAVAILABLE_NV");
//		lua_pushinteger(lua, GL_FONT_UNAVAILABLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_UNINTELLIGIBLE_NV");
//		lua_pushinteger(lua, GL_FONT_UNINTELLIGIBLE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STANDARD_FONT_FORMAT_NV");
//		lua_pushinteger(lua, GL_STANDARD_FONT_FORMAT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_INPUT_NV");
//		lua_pushinteger(lua, GL_FRAGMENT_INPUT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_X_MIN_BOUNDS_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_X_MIN_BOUNDS_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_Y_MIN_BOUNDS_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_Y_MIN_BOUNDS_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_X_MAX_BOUNDS_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_X_MAX_BOUNDS_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_Y_MAX_BOUNDS_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_Y_MAX_BOUNDS_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_UNITS_PER_EM_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_UNITS_PER_EM_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_ASCENDER_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_ASCENDER_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_DESCENDER_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_DESCENDER_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_HEIGHT_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_HEIGHT_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_MAX_ADVANCE_WIDTH_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_MAX_ADVANCE_WIDTH_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_MAX_ADVANCE_HEIGHT_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_MAX_ADVANCE_HEIGHT_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_UNDERLINE_POSITION_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_UNDERLINE_POSITION_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_UNDERLINE_THICKNESS_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_UNDERLINE_THICKNESS_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_HAS_KERNING_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_HAS_KERNING_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FONT_NUM_GLYPH_INDICES_BIT_NV");
//		lua_pushinteger(lua, GL_FONT_NUM_GLYPH_INDICES_BIT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_path_rendering_shared_edge");
//		lua_pushinteger(lua, GL_NV_path_rendering_shared_edge);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SHARED_EDGE_NV");
//		lua_pushinteger(lua, GL_SHARED_EDGE_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_pixel_data_range");
		lua_pushinteger(lua, GL_NV_pixel_data_range);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRITE_PIXEL_DATA_RANGE_NV");
		lua_pushinteger(lua, GL_WRITE_PIXEL_DATA_RANGE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_PIXEL_DATA_RANGE_NV");
		lua_pushinteger(lua, GL_READ_PIXEL_DATA_RANGE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRITE_PIXEL_DATA_RANGE_LENGTH_NV");
		lua_pushinteger(lua, GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_PIXEL_DATA_RANGE_LENGTH_NV");
		lua_pushinteger(lua, GL_READ_PIXEL_DATA_RANGE_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRITE_PIXEL_DATA_RANGE_POINTER_NV");
		lua_pushinteger(lua, GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "READ_PIXEL_DATA_RANGE_POINTER_NV");
		lua_pushinteger(lua, GL_READ_PIXEL_DATA_RANGE_POINTER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_point_sprite");
		lua_pushinteger(lua, GL_NV_point_sprite);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SPRITE_NV");
		lua_pushinteger(lua, GL_POINT_SPRITE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COORD_REPLACE_NV");
		lua_pushinteger(lua, GL_COORD_REPLACE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POINT_SPRITE_R_MODE_NV");
		lua_pushinteger(lua, GL_POINT_SPRITE_R_MODE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_present_video");
		lua_pushinteger(lua, GL_NV_present_video);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FRAME_NV");
		lua_pushinteger(lua, GL_FRAME_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIELDS_NV");
		lua_pushinteger(lua, GL_FIELDS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_TIME_NV");
		lua_pushinteger(lua, GL_CURRENT_TIME_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_FILL_STREAMS_NV");
		lua_pushinteger(lua, GL_NUM_FILL_STREAMS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRESENT_TIME_NV");
		lua_pushinteger(lua, GL_PRESENT_TIME_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRESENT_DURATION_NV");
		lua_pushinteger(lua, GL_PRESENT_DURATION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_primitive_restart");
		lua_pushinteger(lua, GL_NV_primitive_restart);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVE_RESTART_NV");
		lua_pushinteger(lua, GL_PRIMITIVE_RESTART_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVE_RESTART_INDEX_NV");
		lua_pushinteger(lua, GL_PRIMITIVE_RESTART_INDEX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_register_combiners");
		lua_pushinteger(lua, GL_NV_register_combiners);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REGISTER_COMBINERS_NV");
		lua_pushinteger(lua, GL_REGISTER_COMBINERS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_A_NV");
		lua_pushinteger(lua, GL_VARIABLE_A_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_B_NV");
		lua_pushinteger(lua, GL_VARIABLE_B_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_C_NV");
		lua_pushinteger(lua, GL_VARIABLE_C_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_D_NV");
		lua_pushinteger(lua, GL_VARIABLE_D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_E_NV");
		lua_pushinteger(lua, GL_VARIABLE_E_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_F_NV");
		lua_pushinteger(lua, GL_VARIABLE_F_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VARIABLE_G_NV");
		lua_pushinteger(lua, GL_VARIABLE_G_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_COLOR0_NV");
		lua_pushinteger(lua, GL_CONSTANT_COLOR0_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSTANT_COLOR1_NV");
		lua_pushinteger(lua, GL_CONSTANT_COLOR1_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMARY_COLOR_NV");
		lua_pushinteger(lua, GL_PRIMARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_NV");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPARE0_NV");
		lua_pushinteger(lua, GL_SPARE0_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPARE1_NV");
		lua_pushinteger(lua, GL_SPARE1_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DISCARD_NV");
		lua_pushinteger(lua, GL_DISCARD_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "E_TIMES_F_NV");
		lua_pushinteger(lua, GL_E_TIMES_F_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SPARE0_PLUS_SECONDARY_COLOR_NV");
		lua_pushinteger(lua, GL_SPARE0_PLUS_SECONDARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_IDENTITY_NV");
		lua_pushinteger(lua, GL_UNSIGNED_IDENTITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INVERT_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INVERT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXPAND_NORMAL_NV");
		lua_pushinteger(lua, GL_EXPAND_NORMAL_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EXPAND_NEGATE_NV");
		lua_pushinteger(lua, GL_EXPAND_NEGATE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_BIAS_NORMAL_NV");
		lua_pushinteger(lua, GL_HALF_BIAS_NORMAL_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HALF_BIAS_NEGATE_NV");
		lua_pushinteger(lua, GL_HALF_BIAS_NEGATE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_IDENTITY_NV");
		lua_pushinteger(lua, GL_SIGNED_IDENTITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_NEGATE_NV");
		lua_pushinteger(lua, GL_SIGNED_NEGATE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCALE_BY_TWO_NV");
		lua_pushinteger(lua, GL_SCALE_BY_TWO_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCALE_BY_FOUR_NV");
		lua_pushinteger(lua, GL_SCALE_BY_FOUR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCALE_BY_ONE_HALF_NV");
		lua_pushinteger(lua, GL_SCALE_BY_ONE_HALF_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BIAS_BY_NEGATIVE_ONE_HALF_NV");
		lua_pushinteger(lua, GL_BIAS_BY_NEGATIVE_ONE_HALF_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_INPUT_NV");
		lua_pushinteger(lua, GL_COMBINER_INPUT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_MAPPING_NV");
		lua_pushinteger(lua, GL_COMBINER_MAPPING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_COMPONENT_USAGE_NV");
		lua_pushinteger(lua, GL_COMBINER_COMPONENT_USAGE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_AB_DOT_PRODUCT_NV");
		lua_pushinteger(lua, GL_COMBINER_AB_DOT_PRODUCT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_CD_DOT_PRODUCT_NV");
		lua_pushinteger(lua, GL_COMBINER_CD_DOT_PRODUCT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_MUX_SUM_NV");
		lua_pushinteger(lua, GL_COMBINER_MUX_SUM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_SCALE_NV");
		lua_pushinteger(lua, GL_COMBINER_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_BIAS_NV");
		lua_pushinteger(lua, GL_COMBINER_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_AB_OUTPUT_NV");
		lua_pushinteger(lua, GL_COMBINER_AB_OUTPUT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_CD_OUTPUT_NV");
		lua_pushinteger(lua, GL_COMBINER_CD_OUTPUT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER_SUM_OUTPUT_NV");
		lua_pushinteger(lua, GL_COMBINER_SUM_OUTPUT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_GENERAL_COMBINERS_NV");
		lua_pushinteger(lua, GL_MAX_GENERAL_COMBINERS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_GENERAL_COMBINERS_NV");
		lua_pushinteger(lua, GL_NUM_GENERAL_COMBINERS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_SUM_CLAMP_NV");
		lua_pushinteger(lua, GL_COLOR_SUM_CLAMP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER0_NV");
		lua_pushinteger(lua, GL_COMBINER0_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER1_NV");
		lua_pushinteger(lua, GL_COMBINER1_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER2_NV");
		lua_pushinteger(lua, GL_COMBINER2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER3_NV");
		lua_pushinteger(lua, GL_COMBINER3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER4_NV");
		lua_pushinteger(lua, GL_COMBINER4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER5_NV");
		lua_pushinteger(lua, GL_COMBINER5_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER6_NV");
		lua_pushinteger(lua, GL_COMBINER6_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINER7_NV");
		lua_pushinteger(lua, GL_COMBINER7_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_register_combiners2");
		lua_pushinteger(lua, GL_NV_register_combiners2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PER_STAGE_CONSTANTS_NV");
		lua_pushinteger(lua, GL_PER_STAGE_CONSTANTS_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_sample_locations");
//		lua_pushinteger(lua, GL_NV_sample_locations);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLE_LOCATION_NV");
//		lua_pushinteger(lua, GL_SAMPLE_LOCATION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLE_LOCATION_SUBPIXEL_BITS_NV");
//		lua_pushinteger(lua, GL_SAMPLE_LOCATION_SUBPIXEL_BITS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLE_LOCATION_PIXEL_GRID_WIDTH_NV");
//		lua_pushinteger(lua, GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_NV");
//		lua_pushinteger(lua, GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_NV");
//		lua_pushinteger(lua, GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PROGRAMMABLE_SAMPLE_LOCATION_NV");
//		lua_pushinteger(lua, GL_PROGRAMMABLE_SAMPLE_LOCATION_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_NV");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_NV");
//		lua_pushinteger(lua, GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_sample_mask_override_coverage");
//		lua_pushinteger(lua, GL_NV_sample_mask_override_coverage);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_atomic_counters");
//		lua_pushinteger(lua, GL_NV_shader_atomic_counters);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_atomic_float");
//		lua_pushinteger(lua, GL_NV_shader_atomic_float);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_atomic_fp16_vector");
//		lua_pushinteger(lua, GL_NV_shader_atomic_fp16_vector);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_atomic_int64");
//		lua_pushinteger(lua, GL_NV_shader_atomic_int64);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_shader_buffer_load");
		lua_pushinteger(lua, GL_NV_shader_buffer_load);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BUFFER_GPU_ADDRESS_NV");
		lua_pushinteger(lua, GL_BUFFER_GPU_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GPU_ADDRESS_NV");
		lua_pushinteger(lua, GL_GPU_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_SHADER_BUFFER_ADDRESS_NV");
		lua_pushinteger(lua, GL_MAX_SHADER_BUFFER_ADDRESS_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_storage_buffer_object");
//		lua_pushinteger(lua, GL_NV_shader_storage_buffer_object);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_thread_group");
//		lua_pushinteger(lua, GL_NV_shader_thread_group);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "WARP_SIZE_NV");
//		lua_pushinteger(lua, GL_WARP_SIZE_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "WARPS_PER_SM_NV");
//		lua_pushinteger(lua, GL_WARPS_PER_SM_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SM_COUNT_NV");
//		lua_pushinteger(lua, GL_SM_COUNT_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_shader_thread_shuffle");
//		lua_pushinteger(lua, GL_NV_shader_thread_shuffle);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_tessellation_program5");
		lua_pushinteger(lua, GL_NV_tessellation_program5);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_PATCH_ATTRIBS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_PATCH_ATTRIBS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_CONTROL_PROGRAM_NV");
		lua_pushinteger(lua, GL_TESS_CONTROL_PROGRAM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_EVALUATION_PROGRAM_NV");
		lua_pushinteger(lua, GL_TESS_EVALUATION_PROGRAM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV");
		lua_pushinteger(lua, GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV");
		lua_pushinteger(lua, GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texgen_emboss");
		lua_pushinteger(lua, GL_NV_texgen_emboss);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EMBOSS_LIGHT_NV");
		lua_pushinteger(lua, GL_EMBOSS_LIGHT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EMBOSS_CONSTANT_NV");
		lua_pushinteger(lua, GL_EMBOSS_CONSTANT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EMBOSS_MAP_NV");
		lua_pushinteger(lua, GL_EMBOSS_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texgen_reflection");
		lua_pushinteger(lua, GL_NV_texgen_reflection);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_MAP_NV");
		lua_pushinteger(lua, GL_NORMAL_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REFLECTION_MAP_NV");
		lua_pushinteger(lua, GL_REFLECTION_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_barrier");
		lua_pushinteger(lua, GL_NV_texture_barrier);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_compression_vtc");
		lua_pushinteger(lua, GL_NV_texture_compression_vtc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_env_combine4");
		lua_pushinteger(lua, GL_NV_texture_env_combine4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COMBINE4_NV");
		lua_pushinteger(lua, GL_COMBINE4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE3_RGB_NV");
		lua_pushinteger(lua, GL_SOURCE3_RGB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SOURCE3_ALPHA_NV");
		lua_pushinteger(lua, GL_SOURCE3_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND3_RGB_NV");
		lua_pushinteger(lua, GL_OPERAND3_RGB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OPERAND3_ALPHA_NV");
		lua_pushinteger(lua, GL_OPERAND3_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_expand_normal");
		lua_pushinteger(lua, GL_NV_texture_expand_normal);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_UNSIGNED_REMAP_MODE_NV");
		lua_pushinteger(lua, GL_TEXTURE_UNSIGNED_REMAP_MODE_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_texture_multisample");
//		lua_pushinteger(lua, GL_NV_texture_multisample);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_COVERAGE_SAMPLES_NV");
//		lua_pushinteger(lua, GL_TEXTURE_COVERAGE_SAMPLES_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_COLOR_SAMPLES_NV");
//		lua_pushinteger(lua, GL_TEXTURE_COLOR_SAMPLES_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_rectangle");
		lua_pushinteger(lua, GL_NV_texture_rectangle);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BINDING_RECTANGLE_NV");
		lua_pushinteger(lua, GL_TEXTURE_BINDING_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_RECTANGLE_TEXTURE_SIZE_NV");
		lua_pushinteger(lua, GL_MAX_RECTANGLE_TEXTURE_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_shader");
		lua_pushinteger(lua, GL_NV_texture_shader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_RECTANGLE_SCALE_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV");
		lua_pushinteger(lua, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_S8_S8_8_8_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_S8_S8_8_8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_S8_S8_REV_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_S8_S8_REV_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_MAG_INTENSITY_NV");
		lua_pushinteger(lua, GL_DSDT_MAG_INTENSITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_CONSISTENT_NV");
		lua_pushinteger(lua, GL_SHADER_CONSISTENT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_SHADER_NV");
		lua_pushinteger(lua, GL_TEXTURE_SHADER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADER_OPERATION_NV");
		lua_pushinteger(lua, GL_SHADER_OPERATION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_MODES_NV");
		lua_pushinteger(lua, GL_CULL_MODES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_2D_MATRIX_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_2D_MATRIX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_MATRIX_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_MATRIX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_2D_SCALE_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_2D_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_SCALE_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_2D_BIAS_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_2D_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_BIAS_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PREVIOUS_TEXTURE_INPUT_NV");
		lua_pushinteger(lua, GL_PREVIOUS_TEXTURE_INPUT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONST_EYE_NV");
		lua_pushinteger(lua, GL_CONST_EYE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PASS_THROUGH_NV");
		lua_pushinteger(lua, GL_PASS_THROUGH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CULL_FRAGMENT_NV");
		lua_pushinteger(lua, GL_CULL_FRAGMENT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_OFFSET_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPENDENT_AR_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_DEPENDENT_AR_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPENDENT_GB_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_DEPENDENT_GB_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_DEPTH_REPLACE_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_DEPTH_REPLACE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_TEXTURE_CUBE_MAP_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_REFLECT_CUBE_MAP_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HILO_NV");
		lua_pushinteger(lua, GL_HILO_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_NV");
		lua_pushinteger(lua, GL_DSDT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_MAG_NV");
		lua_pushinteger(lua, GL_DSDT_MAG_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_MAG_VIB_NV");
		lua_pushinteger(lua, GL_DSDT_MAG_VIB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HILO16_NV");
		lua_pushinteger(lua, GL_HILO16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_HILO_NV");
		lua_pushinteger(lua, GL_SIGNED_HILO_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_HILO16_NV");
		lua_pushinteger(lua, GL_SIGNED_HILO16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGBA_NV");
		lua_pushinteger(lua, GL_SIGNED_RGBA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGBA8_NV");
		lua_pushinteger(lua, GL_SIGNED_RGBA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB8_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE8_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE_ALPHA_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE8_ALPHA8_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE8_ALPHA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_ALPHA_NV");
		lua_pushinteger(lua, GL_SIGNED_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_ALPHA8_NV");
		lua_pushinteger(lua, GL_SIGNED_ALPHA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_INTENSITY_NV");
		lua_pushinteger(lua, GL_SIGNED_INTENSITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_INTENSITY8_NV");
		lua_pushinteger(lua, GL_SIGNED_INTENSITY8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT8_NV");
		lua_pushinteger(lua, GL_DSDT8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT8_MAG8_NV");
		lua_pushinteger(lua, GL_DSDT8_MAG8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT8_MAG8_INTENSITY8_NV");
		lua_pushinteger(lua, GL_DSDT8_MAG8_INTENSITY8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB_UNSIGNED_ALPHA_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB_UNSIGNED_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB8_UNSIGNED_ALPHA8_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HI_SCALE_NV");
		lua_pushinteger(lua, GL_HI_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LO_SCALE_NV");
		lua_pushinteger(lua, GL_LO_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DS_SCALE_NV");
		lua_pushinteger(lua, GL_DS_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DT_SCALE_NV");
		lua_pushinteger(lua, GL_DT_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAGNITUDE_SCALE_NV");
		lua_pushinteger(lua, GL_MAGNITUDE_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIBRANCE_SCALE_NV");
		lua_pushinteger(lua, GL_VIBRANCE_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HI_BIAS_NV");
		lua_pushinteger(lua, GL_HI_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LO_BIAS_NV");
		lua_pushinteger(lua, GL_LO_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DS_BIAS_NV");
		lua_pushinteger(lua, GL_DS_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DT_BIAS_NV");
		lua_pushinteger(lua, GL_DT_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAGNITUDE_BIAS_NV");
		lua_pushinteger(lua, GL_MAGNITUDE_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIBRANCE_BIAS_NV");
		lua_pushinteger(lua, GL_VIBRANCE_BIAS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BORDER_VALUES_NV");
		lua_pushinteger(lua, GL_TEXTURE_BORDER_VALUES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_HI_SIZE_NV");
		lua_pushinteger(lua, GL_TEXTURE_HI_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LO_SIZE_NV");
		lua_pushinteger(lua, GL_TEXTURE_LO_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DS_SIZE_NV");
		lua_pushinteger(lua, GL_TEXTURE_DS_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_DT_SIZE_NV");
		lua_pushinteger(lua, GL_TEXTURE_DT_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAG_SIZE_NV");
		lua_pushinteger(lua, GL_TEXTURE_MAG_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_shader2");
		lua_pushinteger(lua, GL_NV_texture_shader2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_S8_S8_8_8_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_S8_S8_8_8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT_8_8_S8_S8_REV_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT_8_8_S8_S8_REV_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_MAG_INTENSITY_NV");
		lua_pushinteger(lua, GL_DSDT_MAG_INTENSITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_TEXTURE_3D_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_TEXTURE_3D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HILO_NV");
		lua_pushinteger(lua, GL_HILO_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_NV");
		lua_pushinteger(lua, GL_DSDT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_MAG_NV");
		lua_pushinteger(lua, GL_DSDT_MAG_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT_MAG_VIB_NV");
		lua_pushinteger(lua, GL_DSDT_MAG_VIB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HILO16_NV");
		lua_pushinteger(lua, GL_HILO16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_HILO_NV");
		lua_pushinteger(lua, GL_SIGNED_HILO_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_HILO16_NV");
		lua_pushinteger(lua, GL_SIGNED_HILO16_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGBA_NV");
		lua_pushinteger(lua, GL_SIGNED_RGBA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGBA8_NV");
		lua_pushinteger(lua, GL_SIGNED_RGBA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB8_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE8_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE_ALPHA_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_LUMINANCE8_ALPHA8_NV");
		lua_pushinteger(lua, GL_SIGNED_LUMINANCE8_ALPHA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_ALPHA_NV");
		lua_pushinteger(lua, GL_SIGNED_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_ALPHA8_NV");
		lua_pushinteger(lua, GL_SIGNED_ALPHA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_INTENSITY_NV");
		lua_pushinteger(lua, GL_SIGNED_INTENSITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_INTENSITY8_NV");
		lua_pushinteger(lua, GL_SIGNED_INTENSITY8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT8_NV");
		lua_pushinteger(lua, GL_DSDT8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT8_MAG8_NV");
		lua_pushinteger(lua, GL_DSDT8_MAG8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DSDT8_MAG8_INTENSITY8_NV");
		lua_pushinteger(lua, GL_DSDT8_MAG8_INTENSITY8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB_UNSIGNED_ALPHA_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB_UNSIGNED_ALPHA_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_RGB8_UNSIGNED_ALPHA8_NV");
		lua_pushinteger(lua, GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_texture_shader3");
		lua_pushinteger(lua, GL_NV_texture_shader3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_PROJECTIVE_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV");
		lua_pushinteger(lua, GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV");
		lua_pushinteger(lua, GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_HILO_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_OFFSET_HILO_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_HILO_TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV");
		lua_pushinteger(lua, GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPENDENT_HILO_TEXTURE_2D_NV");
		lua_pushinteger(lua, GL_DEPENDENT_HILO_TEXTURE_2D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPENDENT_RGB_TEXTURE_3D_NV");
		lua_pushinteger(lua, GL_DEPENDENT_RGB_TEXTURE_3D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV");
		lua_pushinteger(lua, GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_PASS_THROUGH_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_PASS_THROUGH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_TEXTURE_1D_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_TEXTURE_1D_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV");
		lua_pushinteger(lua, GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "HILO8_NV");
		lua_pushinteger(lua, GL_HILO8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SIGNED_HILO8_NV");
		lua_pushinteger(lua, GL_SIGNED_HILO8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FORCE_BLUE_TO_ONE_NV");
		lua_pushinteger(lua, GL_FORCE_BLUE_TO_ONE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_transform_feedback");
		lua_pushinteger(lua, GL_NV_transform_feedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_PRIMARY_COLOR_NV");
		lua_pushinteger(lua, GL_BACK_PRIMARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_SECONDARY_COLOR_NV");
		lua_pushinteger(lua, GL_BACK_SECONDARY_COLOR_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_NV");
		lua_pushinteger(lua, GL_TEXTURE_COORD_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_DISTANCE_NV");
		lua_pushinteger(lua, GL_CLIP_DISTANCE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ID_NV");
		lua_pushinteger(lua, GL_VERTEX_ID_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVE_ID_NV");
		lua_pushinteger(lua, GL_PRIMITIVE_ID_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GENERIC_ATTRIB_NV");
		lua_pushinteger(lua, GL_GENERIC_ATTRIB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_ATTRIBS_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_ATTRIBS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_MODE_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_VARYINGS_NV");
		lua_pushinteger(lua, GL_ACTIVE_VARYINGS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ACTIVE_VARYING_MAX_LENGTH_NV");
		lua_pushinteger(lua, GL_ACTIVE_VARYING_MAX_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_VARYINGS_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_VARYINGS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_START_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_START_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_SIZE_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_RECORD_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_RECORD_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PRIMITIVES_GENERATED_NV");
		lua_pushinteger(lua, GL_PRIMITIVES_GENERATED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RASTERIZER_DISCARD_NV");
		lua_pushinteger(lua, GL_RASTERIZER_DISCARD_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV");
//		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV");
		lua_pushinteger(lua, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLEAVED_ATTRIBS_NV");
		lua_pushinteger(lua, GL_INTERLEAVED_ATTRIBS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SEPARATE_ATTRIBS_NV");
		lua_pushinteger(lua, GL_SEPARATE_ATTRIBS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_BINDING_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_transform_feedback2");
		lua_pushinteger(lua, GL_NV_transform_feedback2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSFORM_FEEDBACK_BINDING_NV");
		lua_pushinteger(lua, GL_TRANSFORM_FEEDBACK_BINDING_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_uniform_buffer_unified_memory");
//		lua_pushinteger(lua, GL_NV_uniform_buffer_unified_memory);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_BUFFER_UNIFIED_NV");
//		lua_pushinteger(lua, GL_UNIFORM_BUFFER_UNIFIED_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_BUFFER_ADDRESS_NV");
//		lua_pushinteger(lua, GL_UNIFORM_BUFFER_ADDRESS_NV);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "UNIFORM_BUFFER_LENGTH_NV");
//		lua_pushinteger(lua, GL_UNIFORM_BUFFER_LENGTH_NV);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vdpau_interop");
		lua_pushinteger(lua, GL_NV_vdpau_interop);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SURFACE_STATE_NV");
		lua_pushinteger(lua, GL_SURFACE_STATE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SURFACE_REGISTERED_NV");
		lua_pushinteger(lua, GL_SURFACE_REGISTERED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SURFACE_MAPPED_NV");
		lua_pushinteger(lua, GL_SURFACE_MAPPED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRITE_DISCARD_NV");
		lua_pushinteger(lua, GL_WRITE_DISCARD_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_array_range");
		lua_pushinteger(lua, GL_NV_vertex_array_range);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_LENGTH_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_VALID_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_VALID_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV");
		lua_pushinteger(lua, GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_POINTER_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_POINTER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_array_range2");
		lua_pushinteger(lua, GL_NV_vertex_array_range2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_attrib_integer_64bit");
		lua_pushinteger(lua, GL_NV_vertex_attrib_integer_64bit);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INT64_NV");
		lua_pushinteger(lua, GL_INT64_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNSIGNED_INT64_NV");
		lua_pushinteger(lua, GL_UNSIGNED_INT64_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_buffer_unified_memory");
		lua_pushinteger(lua, GL_NV_vertex_buffer_unified_memory);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_UNIFIED_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_UNIFIED_NV");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_UNIFIED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_COLOR_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_INDEX_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_ADDRESS_NV");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_VERTEX_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_NORMAL_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_COLOR_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_INDEX_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COORD_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_TEXTURE_COORD_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGE_FLAG_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_EDGE_FLAG_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SECONDARY_COLOR_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_SECONDARY_COLOR_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_COORD_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_FOG_COORD_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ELEMENT_ARRAY_LENGTH_NV");
		lua_pushinteger(lua, GL_ELEMENT_ARRAY_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_INDIRECT_UNIFIED_NV");
		lua_pushinteger(lua, GL_DRAW_INDIRECT_UNIFIED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_INDIRECT_ADDRESS_NV");
		lua_pushinteger(lua, GL_DRAW_INDIRECT_ADDRESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DRAW_INDIRECT_LENGTH_NV");
		lua_pushinteger(lua, GL_DRAW_INDIRECT_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_program");
		lua_pushinteger(lua, GL_NV_vertex_program);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_NV");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_STATE_PROGRAM_NV");
		lua_pushinteger(lua, GL_VERTEX_STATE_PROGRAM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTRIB_ARRAY_SIZE_NV");
		lua_pushinteger(lua, GL_ATTRIB_ARRAY_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTRIB_ARRAY_STRIDE_NV");
		lua_pushinteger(lua, GL_ATTRIB_ARRAY_STRIDE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTRIB_ARRAY_TYPE_NV");
		lua_pushinteger(lua, GL_ATTRIB_ARRAY_TYPE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_ATTRIB_NV");
		lua_pushinteger(lua, GL_CURRENT_ATTRIB_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_LENGTH_NV");
		lua_pushinteger(lua, GL_PROGRAM_LENGTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_STRING_NV");
		lua_pushinteger(lua, GL_PROGRAM_STRING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MODELVIEW_PROJECTION_NV");
		lua_pushinteger(lua, GL_MODELVIEW_PROJECTION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IDENTITY_NV");
		lua_pushinteger(lua, GL_IDENTITY_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVERSE_NV");
		lua_pushinteger(lua, GL_INVERSE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRANSPOSE_NV");
		lua_pushinteger(lua, GL_TRANSPOSE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVERSE_TRANSPOSE_NV");
		lua_pushinteger(lua, GL_INVERSE_TRANSPOSE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRACK_MATRIX_STACK_DEPTH_NV");
		lua_pushinteger(lua, GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_TRACK_MATRICES_NV");
		lua_pushinteger(lua, GL_MAX_TRACK_MATRICES_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX0_NV");
		lua_pushinteger(lua, GL_MATRIX0_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX1_NV");
		lua_pushinteger(lua, GL_MATRIX1_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX2_NV");
		lua_pushinteger(lua, GL_MATRIX2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX3_NV");
		lua_pushinteger(lua, GL_MATRIX3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX4_NV");
		lua_pushinteger(lua, GL_MATRIX4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX5_NV");
		lua_pushinteger(lua, GL_MATRIX5_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX6_NV");
		lua_pushinteger(lua, GL_MATRIX6_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATRIX7_NV");
		lua_pushinteger(lua, GL_MATRIX7_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_MATRIX_STACK_DEPTH_NV");
		lua_pushinteger(lua, GL_CURRENT_MATRIX_STACK_DEPTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CURRENT_MATRIX_NV");
		lua_pushinteger(lua, GL_CURRENT_MATRIX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_POINT_SIZE_NV");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_POINT_SIZE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_TWO_SIDE_NV");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_TWO_SIDE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_PARAMETER_NV");
		lua_pushinteger(lua, GL_PROGRAM_PARAMETER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ATTRIB_ARRAY_POINTER_NV");
		lua_pushinteger(lua, GL_ATTRIB_ARRAY_POINTER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_TARGET_NV");
		lua_pushinteger(lua, GL_PROGRAM_TARGET_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_RESIDENT_NV");
		lua_pushinteger(lua, GL_PROGRAM_RESIDENT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRACK_MATRIX_NV");
		lua_pushinteger(lua, GL_TRACK_MATRIX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRACK_MATRIX_TRANSFORM_NV");
		lua_pushinteger(lua, GL_TRACK_MATRIX_TRANSFORM_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PROGRAM_BINDING_NV");
		lua_pushinteger(lua, GL_VERTEX_PROGRAM_BINDING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROGRAM_ERROR_POSITION_NV");
		lua_pushinteger(lua, GL_PROGRAM_ERROR_POSITION_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY0_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY0_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY1_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY1_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY2_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY2_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY3_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY3_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY4_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY5_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY5_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY6_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY6_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY7_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY7_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY8_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY8_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY9_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY9_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY10_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY10_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY11_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY11_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY12_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY12_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY13_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY13_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY14_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY14_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY15_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY15_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB0_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB0_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB1_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB1_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB2_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB2_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB3_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB3_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB4_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB4_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB5_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB5_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB6_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB6_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB7_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB7_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB8_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB8_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB9_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB9_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB10_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB10_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB11_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB11_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB12_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB12_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB13_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB13_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB14_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB14_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP1_VERTEX_ATTRIB15_4_NV");
		lua_pushinteger(lua, GL_MAP1_VERTEX_ATTRIB15_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB0_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB0_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB1_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB1_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB2_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB2_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB3_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB3_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB4_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB4_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB5_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB5_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB6_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB6_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB7_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB7_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB8_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB8_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB9_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB9_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB10_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB10_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB11_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB11_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB12_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB12_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB13_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB13_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB14_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB14_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAP2_VERTEX_ATTRIB15_4_NV");
		lua_pushinteger(lua, GL_MAP2_VERTEX_ATTRIB15_4_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_program1_1");
		lua_pushinteger(lua, GL_NV_vertex_program1_1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_program2");
		lua_pushinteger(lua, GL_NV_vertex_program2);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_program2_option");
		lua_pushinteger(lua, GL_NV_vertex_program2_option);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_EXEC_INSTRUCTIONS_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_PROGRAM_CALL_DEPTH_NV");
		lua_pushinteger(lua, GL_MAX_PROGRAM_CALL_DEPTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_program3");
		lua_pushinteger(lua, GL_NV_vertex_program3);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_vertex_program4");
		lua_pushinteger(lua, GL_NV_vertex_program4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_ATTRIB_ARRAY_INTEGER_NV");
		lua_pushinteger(lua, GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NV_video_capture");
		lua_pushinteger(lua, GL_NV_video_capture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_BUFFER_NV");
		lua_pushinteger(lua, GL_VIDEO_BUFFER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_BUFFER_BINDING_NV");
		lua_pushinteger(lua, GL_VIDEO_BUFFER_BINDING_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIELD_UPPER_NV");
		lua_pushinteger(lua, GL_FIELD_UPPER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FIELD_LOWER_NV");
		lua_pushinteger(lua, GL_FIELD_LOWER_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NUM_VIDEO_CAPTURE_STREAMS_NV");
		lua_pushinteger(lua, GL_NUM_VIDEO_CAPTURE_STREAMS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV");
		lua_pushinteger(lua, GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_CAPTURE_TO_422_SUPPORTED_NV");
		lua_pushinteger(lua, GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LAST_VIDEO_CAPTURE_STATUS_NV");
		lua_pushinteger(lua, GL_LAST_VIDEO_CAPTURE_STATUS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_BUFFER_PITCH_NV");
		lua_pushinteger(lua, GL_VIDEO_BUFFER_PITCH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_COLOR_CONVERSION_MATRIX_NV");
		lua_pushinteger(lua, GL_VIDEO_COLOR_CONVERSION_MATRIX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_COLOR_CONVERSION_MAX_NV");
		lua_pushinteger(lua, GL_VIDEO_COLOR_CONVERSION_MAX_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_COLOR_CONVERSION_MIN_NV");
		lua_pushinteger(lua, GL_VIDEO_COLOR_CONVERSION_MIN_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_COLOR_CONVERSION_OFFSET_NV");
		lua_pushinteger(lua, GL_VIDEO_COLOR_CONVERSION_OFFSET_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_BUFFER_INTERNAL_FORMAT_NV");
		lua_pushinteger(lua, GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PARTIAL_SUCCESS_NV");
		lua_pushinteger(lua, GL_PARTIAL_SUCCESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUCCESS_NV");
		lua_pushinteger(lua, GL_SUCCESS_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FAILURE_NV");
		lua_pushinteger(lua, GL_FAILURE_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "YCBYCR8_422_NV");
		lua_pushinteger(lua, GL_YCBYCR8_422_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "YCBAYCR8A_4224_NV");
		lua_pushinteger(lua, GL_YCBAYCR8A_4224_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Z6Y10Z6CB10Z6Y10Z6CR10_422_NV");
		lua_pushinteger(lua, GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV");
		lua_pushinteger(lua, GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Z4Y12Z4CB12Z4Y12Z4CR12_422_NV");
		lua_pushinteger(lua, GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV");
		lua_pushinteger(lua, GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Z4Y12Z4CB12Z4CR12_444_NV");
		lua_pushinteger(lua, GL_Z4Y12Z4CB12Z4CR12_444_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_CAPTURE_FRAME_WIDTH_NV");
		lua_pushinteger(lua, GL_VIDEO_CAPTURE_FRAME_WIDTH_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_CAPTURE_FRAME_HEIGHT_NV");
		lua_pushinteger(lua, GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV");
		lua_pushinteger(lua, GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV");
		lua_pushinteger(lua, GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VIDEO_CAPTURE_SURFACE_ORIGIN_NV");
		lua_pushinteger(lua, GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "NV_viewport_array2");
//		lua_pushinteger(lua, GL_NV_viewport_array2);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "OES_byte_coordinates");
//		lua_pushinteger(lua, GL_OES_byte_coordinates);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "OES_compressed_paletted_texture");
//		lua_pushinteger(lua, GL_OES_compressed_paletted_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE4_RGB8_OES");
//		lua_pushinteger(lua, GL_PALETTE4_RGB8_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE4_RGBA8_OES");
//		lua_pushinteger(lua, GL_PALETTE4_RGBA8_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE4_R5_G6_B5_OES");
//		lua_pushinteger(lua, GL_PALETTE4_R5_G6_B5_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE4_RGBA4_OES");
//		lua_pushinteger(lua, GL_PALETTE4_RGBA4_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE4_RGB5_A1_OES");
//		lua_pushinteger(lua, GL_PALETTE4_RGB5_A1_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE8_RGB8_OES");
//		lua_pushinteger(lua, GL_PALETTE8_RGB8_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE8_RGBA8_OES");
//		lua_pushinteger(lua, GL_PALETTE8_RGBA8_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE8_R5_G6_B5_OES");
//		lua_pushinteger(lua, GL_PALETTE8_R5_G6_B5_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE8_RGBA4_OES");
//		lua_pushinteger(lua, GL_PALETTE8_RGBA4_OES);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "PALETTE8_RGB5_A1_OES");
//		lua_pushinteger(lua, GL_PALETTE8_RGB5_A1_OES);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "OES_read_format");
		lua_pushinteger(lua, GL_OES_read_format);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMPLEMENTATION_COLOR_READ_TYPE_OES");
		lua_pushinteger(lua, GL_IMPLEMENTATION_COLOR_READ_TYPE_OES);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IMPLEMENTATION_COLOR_READ_FORMAT_OES");
		lua_pushinteger(lua, GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "OES_single_precision");
//		lua_pushinteger(lua, GL_OES_single_precision);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "OML_interlace");
		lua_pushinteger(lua, GL_OML_interlace);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLACE_OML");
		lua_pushinteger(lua, GL_INTERLACE_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLACE_READ_OML");
		lua_pushinteger(lua, GL_INTERLACE_READ_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OML_resample");
		lua_pushinteger(lua, GL_OML_resample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_RESAMPLE_OML");
		lua_pushinteger(lua, GL_PACK_RESAMPLE_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_RESAMPLE_OML");
		lua_pushinteger(lua, GL_UNPACK_RESAMPLE_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_REPLICATE_OML");
		lua_pushinteger(lua, GL_RESAMPLE_REPLICATE_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_ZERO_FILL_OML");
		lua_pushinteger(lua, GL_RESAMPLE_ZERO_FILL_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_AVERAGE_OML");
		lua_pushinteger(lua, GL_RESAMPLE_AVERAGE_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_DECIMATE_OML");
		lua_pushinteger(lua, GL_RESAMPLE_DECIMATE_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OML_subsample");
		lua_pushinteger(lua, GL_OML_subsample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FORMAT_SUBSAMPLE_24_24_OML");
		lua_pushinteger(lua, GL_FORMAT_SUBSAMPLE_24_24_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FORMAT_SUBSAMPLE_244_244_OML");
		lua_pushinteger(lua, GL_FORMAT_SUBSAMPLE_244_244_OML);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PGI_misc_hints");
		lua_pushinteger(lua, GL_PGI_misc_hints);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PREFER_DOUBLEBUFFER_HINT_PGI");
		lua_pushinteger(lua, GL_PREFER_DOUBLEBUFFER_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONSERVE_MEMORY_HINT_PGI");
		lua_pushinteger(lua, GL_CONSERVE_MEMORY_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RECLAIM_MEMORY_HINT_PGI");
		lua_pushinteger(lua, GL_RECLAIM_MEMORY_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NATIVE_GRAPHICS_HANDLE_PGI");
		lua_pushinteger(lua, GL_NATIVE_GRAPHICS_HANDLE_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NATIVE_GRAPHICS_BEGIN_HINT_PGI");
		lua_pushinteger(lua, GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NATIVE_GRAPHICS_END_HINT_PGI");
		lua_pushinteger(lua, GL_NATIVE_GRAPHICS_END_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALWAYS_FAST_HINT_PGI");
		lua_pushinteger(lua, GL_ALWAYS_FAST_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALWAYS_SOFT_HINT_PGI");
		lua_pushinteger(lua, GL_ALWAYS_SOFT_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALLOW_DRAW_OBJ_HINT_PGI");
		lua_pushinteger(lua, GL_ALLOW_DRAW_OBJ_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALLOW_DRAW_WIN_HINT_PGI");
		lua_pushinteger(lua, GL_ALLOW_DRAW_WIN_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALLOW_DRAW_FRG_HINT_PGI");
		lua_pushinteger(lua, GL_ALLOW_DRAW_FRG_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALLOW_DRAW_MEM_HINT_PGI");
		lua_pushinteger(lua, GL_ALLOW_DRAW_MEM_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STRICT_DEPTHFUNC_HINT_PGI");
		lua_pushinteger(lua, GL_STRICT_DEPTHFUNC_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STRICT_LIGHTING_HINT_PGI");
		lua_pushinteger(lua, GL_STRICT_LIGHTING_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "STRICT_SCISSOR_HINT_PGI");
		lua_pushinteger(lua, GL_STRICT_SCISSOR_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FULL_STIPPLE_HINT_PGI");
		lua_pushinteger(lua, GL_FULL_STIPPLE_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_NEAR_HINT_PGI");
		lua_pushinteger(lua, GL_CLIP_NEAR_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLIP_FAR_HINT_PGI");
		lua_pushinteger(lua, GL_CLIP_FAR_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WIDE_LINE_HINT_PGI");
		lua_pushinteger(lua, GL_WIDE_LINE_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BACK_NORMALS_HINT_PGI");
		lua_pushinteger(lua, GL_BACK_NORMALS_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PGI_vertex_hints");
		lua_pushinteger(lua, GL_PGI_vertex_hints);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX23_BIT_PGI");
		lua_pushinteger(lua, GL_VERTEX23_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX4_BIT_PGI");
		lua_pushinteger(lua, GL_VERTEX4_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR3_BIT_PGI");
		lua_pushinteger(lua, GL_COLOR3_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR4_BIT_PGI");
		lua_pushinteger(lua, GL_COLOR4_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EDGEFLAG_BIT_PGI");
		lua_pushinteger(lua, GL_EDGEFLAG_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INDEX_BIT_PGI");
		lua_pushinteger(lua, GL_INDEX_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_AMBIENT_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_AMBIENT_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_DATA_HINT_PGI");
		lua_pushinteger(lua, GL_VERTEX_DATA_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_CONSISTENT_HINT_PGI");
		lua_pushinteger(lua, GL_VERTEX_CONSISTENT_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MATERIAL_SIDE_HINT_PGI");
		lua_pushinteger(lua, GL_MATERIAL_SIDE_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_VERTEX_HINT_PGI");
		lua_pushinteger(lua, GL_MAX_VERTEX_HINT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_AMBIENT_AND_DIFFUSE_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_DIFFUSE_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_DIFFUSE_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_EMISSION_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_EMISSION_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_COLOR_INDEXES_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_COLOR_INDEXES_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_SHININESS_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_SHININESS_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAT_SPECULAR_BIT_PGI");
		lua_pushinteger(lua, GL_MAT_SPECULAR_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NORMAL_BIT_PGI");
		lua_pushinteger(lua, GL_NORMAL_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXCOORD1_BIT_PGI");
		lua_pushinteger(lua, GL_TEXCOORD1_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXCOORD2_BIT_PGI");
		lua_pushinteger(lua, GL_TEXCOORD2_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXCOORD3_BIT_PGI");
		lua_pushinteger(lua, GL_TEXCOORD3_BIT_PGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXCOORD4_BIT_PGI");
		lua_pushinteger(lua, GL_TEXCOORD4_BIT_PGI);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_ES1_0_compatibility");
//		lua_pushinteger(lua, GL_REGAL_ES1_0_compatibility);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_ES1_1_compatibility");
//		lua_pushinteger(lua, GL_REGAL_ES1_1_compatibility);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_enable");
//		lua_pushinteger(lua, GL_REGAL_enable);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ERROR_REGAL");
//		lua_pushinteger(lua, GL_ERROR_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DEBUG_REGAL");
//		lua_pushinteger(lua, GL_DEBUG_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_REGAL");
//		lua_pushinteger(lua, GL_LOG_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EMULATION_REGAL");
//		lua_pushinteger(lua, GL_EMULATION_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "DRIVER_REGAL");
//		lua_pushinteger(lua, GL_DRIVER_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MISSING_REGAL");
//		lua_pushinteger(lua, GL_MISSING_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TRACE_REGAL");
//		lua_pushinteger(lua, GL_TRACE_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CACHE_REGAL");
//		lua_pushinteger(lua, GL_CACHE_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "CODE_REGAL");
//		lua_pushinteger(lua, GL_CODE_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "STATISTICS_REGAL");
//		lua_pushinteger(lua, GL_STATISTICS_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_error_string");
//		lua_pushinteger(lua, GL_REGAL_error_string);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_extension_query");
//		lua_pushinteger(lua, GL_REGAL_extension_query);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_log");
//		lua_pushinteger(lua, GL_REGAL_log);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_ERROR_REGAL");
//		lua_pushinteger(lua, GL_LOG_ERROR_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_WARNING_REGAL");
//		lua_pushinteger(lua, GL_LOG_WARNING_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_INFO_REGAL");
//		lua_pushinteger(lua, GL_LOG_INFO_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_APP_REGAL");
//		lua_pushinteger(lua, GL_LOG_APP_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_DRIVER_REGAL");
//		lua_pushinteger(lua, GL_LOG_DRIVER_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_INTERNAL_REGAL");
//		lua_pushinteger(lua, GL_LOG_INTERNAL_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_DEBUG_REGAL");
//		lua_pushinteger(lua, GL_LOG_DEBUG_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_STATUS_REGAL");
//		lua_pushinteger(lua, GL_LOG_STATUS_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LOG_HTTP_REGAL");
//		lua_pushinteger(lua, GL_LOG_HTTP_REGAL);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "REGAL_proc_address");
//		lua_pushinteger(lua, GL_REGAL_proc_address);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "REND_screen_coordinates");
		lua_pushinteger(lua, GL_REND_screen_coordinates);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SCREEN_COORDINATES_REND");
		lua_pushinteger(lua, GL_SCREEN_COORDINATES_REND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INVERTED_SCREEN_W_REND");
		lua_pushinteger(lua, GL_INVERTED_SCREEN_W_REND);
		lua_settable(lua, -3);
		lua_pushstring(lua, "S3_s3tc");
		lua_pushinteger(lua, GL_S3_s3tc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB_S3TC");
		lua_pushinteger(lua, GL_RGB_S3TC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGB4_S3TC");
		lua_pushinteger(lua, GL_RGB4_S3TC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA_S3TC");
		lua_pushinteger(lua, GL_RGBA_S3TC);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RGBA4_S3TC");
		lua_pushinteger(lua, GL_RGBA4_S3TC);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGBA_DXT5_S3TC");
//		lua_pushinteger(lua, GL_RGBA_DXT5_S3TC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGBA4_DXT5_S3TC");
//		lua_pushinteger(lua, GL_RGBA4_DXT5_S3TC);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIS_color_range");
//		lua_pushinteger(lua, GL_SGIS_color_range);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "EXTENDED_RANGE_SGIS");
//		lua_pushinteger(lua, GL_EXTENDED_RANGE_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_RED_SGIS");
//		lua_pushinteger(lua, GL_MIN_RED_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_RED_SGIS");
//		lua_pushinteger(lua, GL_MAX_RED_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_GREEN_SGIS");
//		lua_pushinteger(lua, GL_MIN_GREEN_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_GREEN_SGIS");
//		lua_pushinteger(lua, GL_MAX_GREEN_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_BLUE_SGIS");
//		lua_pushinteger(lua, GL_MIN_BLUE_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_BLUE_SGIS");
//		lua_pushinteger(lua, GL_MAX_BLUE_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_ALPHA_SGIS");
//		lua_pushinteger(lua, GL_MIN_ALPHA_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_ALPHA_SGIS");
//		lua_pushinteger(lua, GL_MAX_ALPHA_SGIS);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_detail_texture");
		lua_pushinteger(lua, GL_SGIS_detail_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_fog_function");
		lua_pushinteger(lua, GL_SGIS_fog_function);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_generate_mipmap");
		lua_pushinteger(lua, GL_SGIS_generate_mipmap);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GENERATE_MIPMAP_SGIS");
		lua_pushinteger(lua, GL_GENERATE_MIPMAP_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GENERATE_MIPMAP_HINT_SGIS");
		lua_pushinteger(lua, GL_GENERATE_MIPMAP_HINT_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_multisample");
		lua_pushinteger(lua, GL_SGIS_multisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MULTISAMPLE_SGIS");
		lua_pushinteger(lua, GL_MULTISAMPLE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_MASK_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_MASK_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_ALPHA_TO_ONE_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_ALPHA_TO_ONE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_MASK_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "1PASS_SGIS");
		lua_pushinteger(lua, GL_1PASS_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2PASS_0_SGIS");
		lua_pushinteger(lua, GL_2PASS_0_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "2PASS_1_SGIS");
		lua_pushinteger(lua, GL_2PASS_1_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_0_SGIS");
		lua_pushinteger(lua, GL_4PASS_0_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_1_SGIS");
		lua_pushinteger(lua, GL_4PASS_1_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_2_SGIS");
		lua_pushinteger(lua, GL_4PASS_2_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "4PASS_3_SGIS");
		lua_pushinteger(lua, GL_4PASS_3_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_BUFFERS_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_BUFFERS_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLES_SGIS");
		lua_pushinteger(lua, GL_SAMPLES_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_VALUE_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_MASK_VALUE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_MASK_INVERT_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_MASK_INVERT_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SAMPLE_PATTERN_SGIS");
		lua_pushinteger(lua, GL_SAMPLE_PATTERN_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_pixel_texture");
		lua_pushinteger(lua, GL_SGIS_pixel_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_point_line_texgen");
		lua_pushinteger(lua, GL_SGIS_point_line_texgen);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_DISTANCE_TO_POINT_SGIS");
		lua_pushinteger(lua, GL_EYE_DISTANCE_TO_POINT_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_DISTANCE_TO_POINT_SGIS");
		lua_pushinteger(lua, GL_OBJECT_DISTANCE_TO_POINT_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_DISTANCE_TO_LINE_SGIS");
		lua_pushinteger(lua, GL_EYE_DISTANCE_TO_LINE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_DISTANCE_TO_LINE_SGIS");
		lua_pushinteger(lua, GL_OBJECT_DISTANCE_TO_LINE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_POINT_SGIS");
		lua_pushinteger(lua, GL_EYE_POINT_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_POINT_SGIS");
		lua_pushinteger(lua, GL_OBJECT_POINT_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EYE_LINE_SGIS");
		lua_pushinteger(lua, GL_EYE_LINE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "OBJECT_LINE_SGIS");
		lua_pushinteger(lua, GL_OBJECT_LINE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_sharpen_texture");
		lua_pushinteger(lua, GL_SGIS_sharpen_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_texture4D");
		lua_pushinteger(lua, GL_SGIS_texture4D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_texture_border_clamp");
		lua_pushinteger(lua, GL_SGIS_texture_border_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_BORDER_SGIS");
		lua_pushinteger(lua, GL_CLAMP_TO_BORDER_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_texture_edge_clamp");
		lua_pushinteger(lua, GL_SGIS_texture_edge_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CLAMP_TO_EDGE_SGIS");
		lua_pushinteger(lua, GL_CLAMP_TO_EDGE_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_texture_filter4");
		lua_pushinteger(lua, GL_SGIS_texture_filter4);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIS_texture_lod");
		lua_pushinteger(lua, GL_SGIS_texture_lod);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MIN_LOD_SGIS");
		lua_pushinteger(lua, GL_TEXTURE_MIN_LOD_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_LOD_SGIS");
		lua_pushinteger(lua, GL_TEXTURE_MAX_LOD_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_BASE_LEVEL_SGIS");
		lua_pushinteger(lua, GL_TEXTURE_BASE_LEVEL_SGIS);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_LEVEL_SGIS");
		lua_pushinteger(lua, GL_TEXTURE_MAX_LEVEL_SGIS);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIS_texture_select");
//		lua_pushinteger(lua, GL_SGIS_texture_select);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_async");
		lua_pushinteger(lua, GL_SGIX_async);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ASYNC_MARKER_SGIX");
		lua_pushinteger(lua, GL_ASYNC_MARKER_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_async_histogram");
		lua_pushinteger(lua, GL_SGIX_async_histogram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ASYNC_HISTOGRAM_SGIX");
		lua_pushinteger(lua, GL_ASYNC_HISTOGRAM_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ASYNC_HISTOGRAM_SGIX");
		lua_pushinteger(lua, GL_MAX_ASYNC_HISTOGRAM_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_async_pixel");
		lua_pushinteger(lua, GL_SGIX_async_pixel);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ASYNC_TEX_IMAGE_SGIX");
		lua_pushinteger(lua, GL_ASYNC_TEX_IMAGE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ASYNC_DRAW_PIXELS_SGIX");
		lua_pushinteger(lua, GL_ASYNC_DRAW_PIXELS_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ASYNC_READ_PIXELS_SGIX");
		lua_pushinteger(lua, GL_ASYNC_READ_PIXELS_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ASYNC_TEX_IMAGE_SGIX");
		lua_pushinteger(lua, GL_MAX_ASYNC_TEX_IMAGE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ASYNC_DRAW_PIXELS_SGIX");
		lua_pushinteger(lua, GL_MAX_ASYNC_DRAW_PIXELS_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_ASYNC_READ_PIXELS_SGIX");
		lua_pushinteger(lua, GL_MAX_ASYNC_READ_PIXELS_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_blend_alpha_minmax");
		lua_pushinteger(lua, GL_SGIX_blend_alpha_minmax);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_MIN_SGIX");
		lua_pushinteger(lua, GL_ALPHA_MIN_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ALPHA_MAX_SGIX");
		lua_pushinteger(lua, GL_ALPHA_MAX_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_clipmap");
		lua_pushinteger(lua, GL_SGIX_clipmap);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_convolution_accuracy");
		lua_pushinteger(lua, GL_SGIX_convolution_accuracy);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CONVOLUTION_HINT_SGIX");
		lua_pushinteger(lua, GL_CONVOLUTION_HINT_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_depth_texture");
		lua_pushinteger(lua, GL_SGIX_depth_texture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT16_SGIX");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT16_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT24_SGIX");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT24_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DEPTH_COMPONENT32_SGIX");
		lua_pushinteger(lua, GL_DEPTH_COMPONENT32_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_flush_raster");
		lua_pushinteger(lua, GL_SGIX_flush_raster);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_fog_offset");
		lua_pushinteger(lua, GL_SGIX_fog_offset);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_OFFSET_SGIX");
		lua_pushinteger(lua, GL_FOG_OFFSET_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_OFFSET_VALUE_SGIX");
		lua_pushinteger(lua, GL_FOG_OFFSET_VALUE_SGIX);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIX_fog_texture");
//		lua_pushinteger(lua, GL_SGIX_fog_texture);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FOG_PATCHY_FACTOR_SGIX");
//		lua_pushinteger(lua, GL_FOG_PATCHY_FACTOR_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "FRAGMENT_FOG_SGIX");
//		lua_pushinteger(lua, GL_FRAGMENT_FOG_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "TEXTURE_FOG_SGIX");
//		lua_pushinteger(lua, GL_TEXTURE_FOG_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIX_fragment_specular_lighting");
//		lua_pushinteger(lua, GL_SGIX_fragment_specular_lighting);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_framezoom");
		lua_pushinteger(lua, GL_SGIX_framezoom);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_interlace");
		lua_pushinteger(lua, GL_SGIX_interlace);
		lua_settable(lua, -3);
		lua_pushstring(lua, "INTERLACE_SGIX");
		lua_pushinteger(lua, GL_INTERLACE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_ir_instrument1");
		lua_pushinteger(lua, GL_SGIX_ir_instrument1);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_list_priority");
		lua_pushinteger(lua, GL_SGIX_list_priority);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_pixel_texture");
		lua_pushinteger(lua, GL_SGIX_pixel_texture);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIX_pixel_texture_bits");
//		lua_pushinteger(lua, GL_SGIX_pixel_texture_bits);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_reference_plane");
		lua_pushinteger(lua, GL_SGIX_reference_plane);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_resample");
		lua_pushinteger(lua, GL_SGIX_resample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PACK_RESAMPLE_SGIX");
		lua_pushinteger(lua, GL_PACK_RESAMPLE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_RESAMPLE_SGIX");
		lua_pushinteger(lua, GL_UNPACK_RESAMPLE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_DECIMATE_SGIX");
		lua_pushinteger(lua, GL_RESAMPLE_DECIMATE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_REPLICATE_SGIX");
		lua_pushinteger(lua, GL_RESAMPLE_REPLICATE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESAMPLE_ZERO_FILL_SGIX");
		lua_pushinteger(lua, GL_RESAMPLE_ZERO_FILL_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_shadow");
		lua_pushinteger(lua, GL_SGIX_shadow);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COMPARE_OPERATOR_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_COMPARE_OPERATOR_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_LEQUAL_R_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_LEQUAL_R_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_GEQUAL_R_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_GEQUAL_R_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_shadow_ambient");
		lua_pushinteger(lua, GL_SGIX_shadow_ambient);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SHADOW_AMBIENT_SGIX");
		lua_pushinteger(lua, GL_SHADOW_AMBIENT_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_sprite");
		lua_pushinteger(lua, GL_SGIX_sprite);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_tag_sample_buffer");
		lua_pushinteger(lua, GL_SGIX_tag_sample_buffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_texture_add_env");
		lua_pushinteger(lua, GL_SGIX_texture_add_env);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_texture_coordinate_clamp");
		lua_pushinteger(lua, GL_SGIX_texture_coordinate_clamp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_CLAMP_S_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_MAX_CLAMP_S_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_CLAMP_T_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_MAX_CLAMP_T_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MAX_CLAMP_R_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_MAX_CLAMP_R_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_texture_lod_bias");
		lua_pushinteger(lua, GL_SGIX_texture_lod_bias);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_texture_multi_buffer");
		lua_pushinteger(lua, GL_SGIX_texture_multi_buffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_MULTI_BUFFER_HINT_SGIX");
		lua_pushinteger(lua, GL_TEXTURE_MULTI_BUFFER_HINT_SGIX);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIX_texture_range");
//		lua_pushinteger(lua, GL_SGIX_texture_range);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGB_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_RGB_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGBA_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_RGBA_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ALPHA_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_ALPHA_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTENSITY_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_INTENSITY_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE_ALPHA_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGB16_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_RGB16_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGBA16_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_RGBA16_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ALPHA16_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_ALPHA16_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE16_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE16_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTENSITY16_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_INTENSITY16_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE16_ALPHA16_SIGNED_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE16_ALPHA16_SIGNED_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGB_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_RGB_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGBA_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_RGBA_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ALPHA_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_ALPHA_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTENSITY_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_INTENSITY_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE_ALPHA_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE_ALPHA_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGB16_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_RGB16_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "RGBA16_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_RGBA16_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "ALPHA16_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_ALPHA16_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE16_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE16_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "INTENSITY16_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_INTENSITY16_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "LUMINANCE16_ALPHA16_EXTENDED_RANGE_SGIX");
//		lua_pushinteger(lua, GL_LUMINANCE16_ALPHA16_EXTENDED_RANGE_SGIX);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_LUMINANCE_SGIS");
//		lua_pushinteger(lua, GL_MIN_LUMINANCE_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_LUMINANCE_SGIS");
//		lua_pushinteger(lua, GL_MAX_LUMINANCE_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MIN_INTENSITY_SGIS");
//		lua_pushinteger(lua, GL_MIN_INTENSITY_SGIS);
//		lua_settable(lua, -3);
//		lua_pushstring(lua, "MAX_INTENSITY_SGIS");
//		lua_pushinteger(lua, GL_MAX_INTENSITY_SGIS);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_texture_scale_bias");
		lua_pushinteger(lua, GL_SGIX_texture_scale_bias);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_TEXTURE_FILTER_BIAS_SGIX");
		lua_pushinteger(lua, GL_POST_TEXTURE_FILTER_BIAS_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_TEXTURE_FILTER_SCALE_SGIX");
		lua_pushinteger(lua, GL_POST_TEXTURE_FILTER_SCALE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_TEXTURE_FILTER_BIAS_RANGE_SGIX");
		lua_pushinteger(lua, GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_TEXTURE_FILTER_SCALE_RANGE_SGIX");
		lua_pushinteger(lua, GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_vertex_preclip");
		lua_pushinteger(lua, GL_SGIX_vertex_preclip);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PRECLIP_SGIX");
		lua_pushinteger(lua, GL_VERTEX_PRECLIP_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PRECLIP_HINT_SGIX");
		lua_pushinteger(lua, GL_VERTEX_PRECLIP_HINT_SGIX);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SGIX_vertex_preclip_hint");
//		lua_pushinteger(lua, GL_SGIX_vertex_preclip_hint);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PRECLIP_SGIX");
		lua_pushinteger(lua, GL_VERTEX_PRECLIP_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VERTEX_PRECLIP_HINT_SGIX");
		lua_pushinteger(lua, GL_VERTEX_PRECLIP_HINT_SGIX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGIX_ycrcb");
		lua_pushinteger(lua, GL_SGIX_ycrcb);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGI_color_matrix");
		lua_pushinteger(lua, GL_SGI_color_matrix);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATRIX_SGI");
		lua_pushinteger(lua, GL_COLOR_MATRIX_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_MATRIX_STACK_DEPTH_SGI");
		lua_pushinteger(lua, GL_COLOR_MATRIX_STACK_DEPTH_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MAX_COLOR_MATRIX_STACK_DEPTH_SGI");
		lua_pushinteger(lua, GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_RED_SCALE_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_RED_SCALE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_GREEN_SCALE_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_BLUE_SCALE_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_ALPHA_SCALE_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_RED_BIAS_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_RED_BIAS_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_GREEN_BIAS_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_BLUE_BIAS_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_ALPHA_BIAS_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGI_color_table");
		lua_pushinteger(lua, GL_SGI_color_table);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_CONVOLUTION_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_POST_CONVOLUTION_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "POST_COLOR_MATRIX_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_PROXY_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_SCALE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_SCALE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_BIAS_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_BIAS_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_FORMAT_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_FORMAT_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_WIDTH_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_WIDTH_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_RED_SIZE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_RED_SIZE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_GREEN_SIZE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_GREEN_SIZE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_BLUE_SIZE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_BLUE_SIZE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_ALPHA_SIZE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_ALPHA_SIZE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_LUMINANCE_SIZE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_LUMINANCE_SIZE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "COLOR_TABLE_INTENSITY_SIZE_SGI");
		lua_pushinteger(lua, GL_COLOR_TABLE_INTENSITY_SIZE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SGI_texture_color_table");
		lua_pushinteger(lua, GL_SGI_texture_color_table);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_TEXTURE_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PROXY_TEXTURE_COLOR_TABLE_SGI");
		lua_pushinteger(lua, GL_PROXY_TEXTURE_COLOR_TABLE_SGI);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUNX_constant_data");
		lua_pushinteger(lua, GL_SUNX_constant_data);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UNPACK_CONSTANT_DATA_SUNX");
		lua_pushinteger(lua, GL_UNPACK_CONSTANT_DATA_SUNX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TEXTURE_CONSTANT_DATA_SUNX");
		lua_pushinteger(lua, GL_TEXTURE_CONSTANT_DATA_SUNX);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUN_convolution_border_modes");
		lua_pushinteger(lua, GL_SUN_convolution_border_modes);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WRAP_BORDER_SUN");
		lua_pushinteger(lua, GL_WRAP_BORDER_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUN_global_alpha");
		lua_pushinteger(lua, GL_SUN_global_alpha);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GLOBAL_ALPHA_SUN");
		lua_pushinteger(lua, GL_GLOBAL_ALPHA_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GLOBAL_ALPHA_FACTOR_SUN");
		lua_pushinteger(lua, GL_GLOBAL_ALPHA_FACTOR_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUN_mesh_array");
		lua_pushinteger(lua, GL_SUN_mesh_array);
		lua_settable(lua, -3);
		lua_pushstring(lua, "QUAD_MESH_SUN");
		lua_pushinteger(lua, GL_QUAD_MESH_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_MESH_SUN");
		lua_pushinteger(lua, GL_TRIANGLE_MESH_SUN);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "SUN_read_video_pixels");
//		lua_pushinteger(lua, GL_SUN_read_video_pixels);
//		lua_settable(lua, -3);
		lua_pushstring(lua, "SUN_slice_accum");
		lua_pushinteger(lua, GL_SUN_slice_accum);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SLICE_ACCUM_SUN");
		lua_pushinteger(lua, GL_SLICE_ACCUM_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUN_triangle_list");
		lua_pushinteger(lua, GL_SUN_triangle_list);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RESTART_SUN");
		lua_pushinteger(lua, GL_RESTART_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACE_MIDDLE_SUN");
		lua_pushinteger(lua, GL_REPLACE_MIDDLE_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACE_OLDEST_SUN");
		lua_pushinteger(lua, GL_REPLACE_OLDEST_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TRIANGLE_LIST_SUN");
		lua_pushinteger(lua, GL_TRIANGLE_LIST_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACEMENT_CODE_SUN");
		lua_pushinteger(lua, GL_REPLACEMENT_CODE_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACEMENT_CODE_ARRAY_SUN");
		lua_pushinteger(lua, GL_REPLACEMENT_CODE_ARRAY_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACEMENT_CODE_ARRAY_TYPE_SUN");
		lua_pushinteger(lua, GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACEMENT_CODE_ARRAY_STRIDE_SUN");
		lua_pushinteger(lua, GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "REPLACEMENT_CODE_ARRAY_POINTER_SUN");
		lua_pushinteger(lua, GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_C4UB_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_C4UB_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_C3F_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_C3F_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_N3F_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_N3F_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_C4F_N3F_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_C4F_N3F_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_T2F_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_T2F_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_T2F_N3F_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_T2F_N3F_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "R1UI_T2F_C4F_N3F_V3F_SUN");
		lua_pushinteger(lua, GL_R1UI_T2F_C4F_N3F_V3F_SUN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SUN_vertex");
		lua_pushinteger(lua, GL_SUN_vertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WIN_phong_shading");
		lua_pushinteger(lua, GL_WIN_phong_shading);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PHONG_WIN");
		lua_pushinteger(lua, GL_PHONG_WIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PHONG_HINT_WIN");
		lua_pushinteger(lua, GL_PHONG_HINT_WIN);
		lua_settable(lua, -3);
		lua_pushstring(lua, "WIN_specular_fog");
		lua_pushinteger(lua, GL_WIN_specular_fog);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FOG_SPECULAR_TEXTURE_WIN");
		lua_pushinteger(lua, GL_FOG_SPECULAR_TEXTURE_WIN);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "WIN_swap_hint");
//		lua_pushinteger(lua, GL_WIN_swap_hint);
//		lua_settable(lua, -3);

		// Functions.
		// Custom.
		lua_pushstring(lua, "DataToTable");
		lua_pushcfunction(lua, lua_glDataToTable);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TableToData");
		lua_pushcfunction(lua, lua_glTableToData);
		lua_settable(lua, -3);

		lua_pushstring(lua, "Enable");
		lua_pushcfunction(lua, lua_glEnable);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Disable");
		lua_pushcfunction(lua, lua_glDisable);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsEnabled");
		lua_pushcfunction(lua, lua_glIsEnabled);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Enablei");
		lua_pushcfunction(lua, lua_glEnablei);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Disablei");
		lua_pushcfunction(lua, lua_glDisablei);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsEnabledi");
		lua_pushcfunction(lua, lua_glIsEnabledi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenBuffer");
		lua_pushcfunction(lua, lua_glGenBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenBuffers");
		lua_pushcfunction(lua, lua_glGenBuffers);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateBuffer");
		// lua_pushcfunction(lua, lua_glCreateBuffer);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateBuffers");
		// lua_pushcfunction(lua, lua_glCreateBuffers);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteBuffer");
		lua_pushcfunction(lua, lua_glDeleteBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteBuffers");
		lua_pushcfunction(lua, lua_glDeleteBuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindBuffer");
		lua_pushcfunction(lua, lua_glBindBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindBufferRange");
		lua_pushcfunction(lua, lua_glBindBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindBufferBase");
		lua_pushcfunction(lua, lua_glBindBufferBase);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindBufferRange");
		lua_pushcfunction(lua, lua_glBindBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindBufferBase");
		lua_pushcfunction(lua, lua_glBindBufferBase);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BufferStorage");
		lua_pushcfunction(lua, lua_glBufferStorage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedBufferStorage");
		lua_pushcfunction(lua, lua_glNamedBufferStorage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BufferData");
		lua_pushcfunction(lua, lua_glBufferData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedBufferData");
		lua_pushcfunction(lua, lua_glNamedBufferData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BufferSubData");
		lua_pushcfunction(lua, lua_glBufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedBufferSubData");
		lua_pushcfunction(lua, lua_glNamedBufferSubData);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "ClearBufferSubData");
		// lua_pushcfunction(lua, lua_glClearBufferSubData);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "ClearNamedBufferSubData");
		lua_pushcfunction(lua, lua_glClearNamedBufferSubData);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "ClearBufferData");
		// lua_pushcfunction(lua, lua_glClearBufferData);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "ClearNamedBufferData");
		lua_pushcfunction(lua, lua_glClearNamedBufferData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MapBufferRange");
		lua_pushcfunction(lua, lua_glMapBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MapNamedBufferRange");
		lua_pushcfunction(lua, lua_glMapNamedBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MapBuffer");
		lua_pushcfunction(lua, lua_glMapBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MapNamedBuffer");
		lua_pushcfunction(lua, lua_glMapNamedBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FlushMappedBufferRange");
		lua_pushcfunction(lua, lua_glFlushMappedBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FlushMappedNamedBufferRange");
		lua_pushcfunction(lua, lua_glFlushMappedNamedBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UnmapBuffer");
		lua_pushcfunction(lua, lua_glUnmapBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UnmapNamedBuffer");
		lua_pushcfunction(lua, lua_glUnmapNamedBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidadeBufferSubData");
		lua_pushcfunction(lua, lua_glInvalidadeBufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidadeBufferData");
		lua_pushcfunction(lua, lua_glInvalidadeBufferData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsBuffer");
		lua_pushcfunction(lua, lua_glIsBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetBufferSubData");
		lua_pushcfunction(lua, lua_glGetBufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetNamedBufferSubData");
		lua_pushcfunction(lua, lua_glGetNamedBufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetBufferParameteriv");
		lua_pushcfunction(lua, lua_glGetBufferParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetNamedBufferParameteriv");
		lua_pushcfunction(lua, lua_glGetNamedBufferParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BufferPointerv");
		lua_pushcfunction(lua, lua_glBufferPointerv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyBufferSubData");
		lua_pushcfunction(lua, lua_glCopyBufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyNamedBufferSubData");
		lua_pushcfunction(lua, lua_glCopyNamedBufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CreateShader");
		lua_pushcfunction(lua, lua_glCreateShader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ShaderSource");
		lua_pushcfunction(lua, lua_glShaderSource);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompileShader");
		lua_pushcfunction(lua, lua_glCompileShader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ReleaseShaderCompiler");
		lua_pushcfunction(lua, lua_glReleaseShaderCompiler);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteShader");
		lua_pushcfunction(lua, lua_glDeleteShader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsShader");
		lua_pushcfunction(lua, lua_glIsShader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ShaderBinary");
		lua_pushcfunction(lua, lua_glShaderBinary);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CreateProgram");
		lua_pushcfunction(lua, lua_glCreateProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AttachShader");
		lua_pushcfunction(lua, lua_glAttachShader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DetachShader");
		lua_pushcfunction(lua, lua_glDetachShader);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LinkProgram");
		lua_pushcfunction(lua, lua_glLinkProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UseProgram");
		lua_pushcfunction(lua, lua_glUseProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteProgram");
		lua_pushcfunction(lua, lua_glDeleteProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsProgram");
		lua_pushcfunction(lua, lua_glIsProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenProgramPipeline");
		lua_pushcfunction(lua, lua_glGenProgramPipeline);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenProgramPipelines");
		lua_pushcfunction(lua, lua_glGenProgramPipelines);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteProgramPipeline");
		lua_pushcfunction(lua, lua_glDeleteProgramPipeline);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsProgramPipeline");
		lua_pushcfunction(lua, lua_glIsProgramPipeline);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateProgramPipelines");
		// lua_pushcfunction(lua, lua_glCreateProgramPipelines);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "ActiveShaderProgram");
		lua_pushcfunction(lua, lua_glActiveShaderProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformLocation");
		lua_pushcfunction(lua, lua_glGetUniformLocation);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveUniformName");
		lua_pushcfunction(lua, lua_glGetActiveUniformName);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformIndices");
		lua_pushcfunction(lua, lua_glGetUniformIndices);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveUniform");
		lua_pushcfunction(lua, lua_glGetActiveUniform);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveUniformsiv");
		lua_pushcfunction(lua, lua_glGetActiveUniformsiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformBlockIndex");
		lua_pushcfunction(lua, lua_glGetUniformBlockIndex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveUniformBlockName");
		lua_pushcfunction(lua, lua_glGetActiveUniformBlockName);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveUniformBlockiv");
		lua_pushcfunction(lua, lua_glGetActiveUniformBlockiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveAtomicCounterBufferiv");
		lua_pushcfunction(lua, lua_glGetActiveAtomicCounterBufferiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1i");
		lua_pushcfunction(lua, lua_glUniform1i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2i");
		lua_pushcfunction(lua, lua_glUniform2i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3i");
		lua_pushcfunction(lua, lua_glUniform3i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4i");
		lua_pushcfunction(lua, lua_glUniform4i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1f");
		lua_pushcfunction(lua, lua_glUniform1f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2f");
		lua_pushcfunction(lua, lua_glUniform2f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3f");
		lua_pushcfunction(lua, lua_glUniform3f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4f");
		lua_pushcfunction(lua, lua_glUniform4f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1d");
		lua_pushcfunction(lua, lua_glUniform1d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2d");
		lua_pushcfunction(lua, lua_glUniform2d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3d");
		lua_pushcfunction(lua, lua_glUniform3d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4d");
		lua_pushcfunction(lua, lua_glUniform4d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1ui");
		lua_pushcfunction(lua, lua_glUniform1ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2ui");
		lua_pushcfunction(lua, lua_glUniform2ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3ui");
		lua_pushcfunction(lua, lua_glUniform3ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4ui");
		lua_pushcfunction(lua, lua_glUniform4ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1iv");
		lua_pushcfunction(lua, lua_glUniform1iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2iv");
		lua_pushcfunction(lua, lua_glUniform2iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3iv");
		lua_pushcfunction(lua, lua_glUniform3iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4iv");
		lua_pushcfunction(lua, lua_glUniform4iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1fv");
		lua_pushcfunction(lua, lua_glUniform1fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2fv");
		lua_pushcfunction(lua, lua_glUniform2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3fv");
		lua_pushcfunction(lua, lua_glUniform3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4fv");
		lua_pushcfunction(lua, lua_glUniform4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1dv");
		lua_pushcfunction(lua, lua_glUniform1dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2dv");
		lua_pushcfunction(lua, lua_glUniform2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3dv");
		lua_pushcfunction(lua, lua_glUniform3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4dv");
		lua_pushcfunction(lua, lua_glUniform4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform1uiv");
		lua_pushcfunction(lua, lua_glUniform1uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform2uiv");
		lua_pushcfunction(lua, lua_glUniform2uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform3uiv");
		lua_pushcfunction(lua, lua_glUniform3uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Uniform4uiv");
		lua_pushcfunction(lua, lua_glUniform4uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix2fv");
		lua_pushcfunction(lua, lua_glUniformMatrix2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix3fv");
		lua_pushcfunction(lua, lua_glUniformMatrix3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix4fv");
		lua_pushcfunction(lua, lua_glUniformMatrix4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix2dv");
		lua_pushcfunction(lua, lua_glUniformMatrix2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix3dv");
		lua_pushcfunction(lua, lua_glUniformMatrix3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix4dv");
		lua_pushcfunction(lua, lua_glUniformMatrix4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix2X3fv");
		lua_pushcfunction(lua, lua_glUniformMatrix2X3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix3X2fv");
		lua_pushcfunction(lua, lua_glUniformMatrix3X2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix2X4fv");
		lua_pushcfunction(lua, lua_glUniformMatrix2X4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix4X2fv");
		lua_pushcfunction(lua, lua_glUniformMatrix4X2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix3X4fv");
		lua_pushcfunction(lua, lua_glUniformMatrix3X4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix4X3fv");
		lua_pushcfunction(lua, lua_glUniformMatrix4X3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix2X3dv");
		lua_pushcfunction(lua, lua_glUniformMatrix2X3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix3X2dv");
		lua_pushcfunction(lua, lua_glUniformMatrix3X2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix2X4dv");
		lua_pushcfunction(lua, lua_glUniformMatrix2X4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix4X2dv");
		lua_pushcfunction(lua, lua_glUniformMatrix4X2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix3X4dv");
		lua_pushcfunction(lua, lua_glUniformMatrix3X4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformMatrix4X3dv");
		lua_pushcfunction(lua, lua_glUniformMatrix4X3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1i");
		lua_pushcfunction(lua, lua_glProgramUniform1i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2i");
		lua_pushcfunction(lua, lua_glProgramUniform2i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3i");
		lua_pushcfunction(lua, lua_glProgramUniform3i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4i");
		lua_pushcfunction(lua, lua_glProgramUniform4i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1f");
		lua_pushcfunction(lua, lua_glProgramUniform1f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2f");
		lua_pushcfunction(lua, lua_glProgramUniform2f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3f");
		lua_pushcfunction(lua, lua_glProgramUniform3f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4f");
		lua_pushcfunction(lua, lua_glProgramUniform4f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1d");
		lua_pushcfunction(lua, lua_glProgramUniform1d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2d");
		lua_pushcfunction(lua, lua_glProgramUniform2d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3d");
		lua_pushcfunction(lua, lua_glProgramUniform3d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4d");
		lua_pushcfunction(lua, lua_glProgramUniform4d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1ui");
		lua_pushcfunction(lua, lua_glProgramUniform1ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2ui");
		lua_pushcfunction(lua, lua_glProgramUniform2ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3ui");
		lua_pushcfunction(lua, lua_glProgramUniform3ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4ui");
		lua_pushcfunction(lua, lua_glProgramUniform4ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1iv");
		lua_pushcfunction(lua, lua_glProgramUniform1iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2iv");
		lua_pushcfunction(lua, lua_glProgramUniform2iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3iv");
		lua_pushcfunction(lua, lua_glProgramUniform3iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4iv");
		lua_pushcfunction(lua, lua_glProgramUniform4iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1fv");
		lua_pushcfunction(lua, lua_glProgramUniform1fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2fv");
		lua_pushcfunction(lua, lua_glProgramUniform2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3fv");
		lua_pushcfunction(lua, lua_glProgramUniform3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4fv");
		lua_pushcfunction(lua, lua_glProgramUniform4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1dv");
		lua_pushcfunction(lua, lua_glProgramUniform1dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2dv");
		lua_pushcfunction(lua, lua_glProgramUniform2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3dv");
		lua_pushcfunction(lua, lua_glProgramUniform3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4dv");
		lua_pushcfunction(lua, lua_glProgramUniform4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform1uiv");
		lua_pushcfunction(lua, lua_glProgramUniform1uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform2uiv");
		lua_pushcfunction(lua, lua_glProgramUniform2uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform3uiv");
		lua_pushcfunction(lua, lua_glProgramUniform3uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniform4uiv");
		lua_pushcfunction(lua, lua_glProgramUniform4uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix3fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix4fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix2dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix3dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix4dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix2X3fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix2X3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix3X2fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix3X2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix2X4fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix2X4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix4X2fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix4X2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix3X4fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix3X4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix4X3fv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix4X3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix2X3dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix2X3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix3X2dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix3X2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix2X4dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix2X4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix4X2dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix4X2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix3X4dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix3X4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramUniformMatrix4X3dv");
		lua_pushcfunction(lua, lua_glProgramUniformMatrix4X3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformBinding");
		lua_pushcfunction(lua, lua_glUniformBinding);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ShaderStorageBlockBindgins");
		lua_pushcfunction(lua, lua_glShaderStorageBlockBindgins);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetSubroutineUniformLocation");
		lua_pushcfunction(lua, lua_glGetSubroutineUniformLocation);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetSubroutineIndex");
		lua_pushcfunction(lua, lua_glGetSubroutineIndex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveSubroutineName");
		lua_pushcfunction(lua, lua_glGetActiveSubroutineName);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ActiveSubroutineUniformiv");
		lua_pushcfunction(lua, lua_glActiveSubroutineUniformiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "UniformSubroutinesuiv");
		lua_pushcfunction(lua, lua_glUniformSubroutinesuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MemoryBarrier");
		lua_pushcfunction(lua, lua_glMemoryBarrier);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MemoryBarrierByRegion");
		lua_pushcfunction(lua, lua_glMemoryBarrierByRegion);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetShaderiv");
		lua_pushcfunction(lua, lua_glGetShaderiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetProgramiv");
		lua_pushcfunction(lua, lua_glGetProgramiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetProgramPipelineiv");
		lua_pushcfunction(lua, lua_glGetProgramPipelineiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "AttachedShaders");
		lua_pushcfunction(lua, lua_glAttachedShaders);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetShaderInfoLog");
		lua_pushcfunction(lua, lua_glGetShaderInfoLog);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramInfoLog");
		lua_pushcfunction(lua, lua_glProgramInfoLog);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProgramPipelineInfoLog");
		lua_pushcfunction(lua, lua_glProgramPipelineInfoLog);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetShaderSource");
		lua_pushcfunction(lua, lua_glGetShaderSource);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetShaderPrecisionFormat");
		lua_pushcfunction(lua, lua_glGetShaderPrecisionFormat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformi");
		lua_pushcfunction(lua, lua_glGetUniformi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformf");
		lua_pushcfunction(lua, lua_glGetUniformf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformd");
		lua_pushcfunction(lua, lua_glGetUniformd);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformui");
		lua_pushcfunction(lua, lua_glGetUniformui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetnUniformi");
		lua_pushcfunction(lua, lua_glGetnUniformi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetnUniformf");
		lua_pushcfunction(lua, lua_glGetnUniformf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetnUniformd");
		lua_pushcfunction(lua, lua_glGetnUniformd);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetnUniformui");
		lua_pushcfunction(lua, lua_glGetnUniformui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetUniformSubroutineuiv");
		lua_pushcfunction(lua, lua_glGetUniformSubroutineuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetProgramStageiv");
		lua_pushcfunction(lua, lua_glGetProgramStageiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ActiveTexture");
		lua_pushcfunction(lua, lua_glActiveTexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenTexture");
		lua_pushcfunction(lua, lua_glGenTexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenTextures");
		lua_pushcfunction(lua, lua_glGenTextures);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindTexture");
		lua_pushcfunction(lua, lua_glBindTexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindTextures");
		lua_pushcfunction(lua, lua_glBindTextures);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "BindTextureUnit");
		// lua_pushcfunction(lua, lua_glBindTextureUnit);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "CreateTextures");
		lua_pushcfunction(lua, lua_glCreateTextures);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteTextures");
		lua_pushcfunction(lua, lua_glDeleteTextures);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsTexture");
		lua_pushcfunction(lua, lua_glIsTexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenSampler");
		lua_pushcfunction(lua, lua_glGenSampler);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenSamplers");
		lua_pushcfunction(lua, lua_glGenSamplers);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateSampler");
		// lua_pushcfunction(lua, lua_glCreateSampler);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateSamplers");
		// lua_pushcfunction(lua, lua_glCreateSamplers);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "BindSampler");
		lua_pushcfunction(lua, lua_glBindSampler);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindSamplers");
		lua_pushcfunction(lua, lua_glBindSamplers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SamplerParameteri");
		lua_pushcfunction(lua, lua_glSamplerParameteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SamplerParameterf");
		lua_pushcfunction(lua, lua_glSamplerParameterf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SamplerParameteriv");
		lua_pushcfunction(lua, lua_glSamplerParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SamplerParameterfv");
		lua_pushcfunction(lua, lua_glSamplerParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SamplerParameterIiv");
		lua_pushcfunction(lua, lua_glSamplerParameterIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SamplerParameterIuiv");
		lua_pushcfunction(lua, lua_glSamplerParameterIuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteSamplers");
		lua_pushcfunction(lua, lua_glDeleteSamplers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsSampler");
		lua_pushcfunction(lua, lua_glIsSampler);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetSamplerParameteriv");
		lua_pushcfunction(lua, lua_glGetSamplerParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetSamplerParamenterfv");
		lua_pushcfunction(lua, lua_glGetSamplerParamenterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetSamplerParameterIiv");
		lua_pushcfunction(lua, lua_glGetSamplerParameterIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetSamplerParameterIfv");
		lua_pushcfunction(lua, lua_glGetSamplerParameterIfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PixelStorei");
		lua_pushcfunction(lua, lua_glPixelStorei);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PixelStoref");
		lua_pushcfunction(lua, lua_glPixelStoref);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexImage3D");
		lua_pushcfunction(lua, lua_glTexImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexImage2D");
		lua_pushcfunction(lua, lua_glTexImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexImage1D");
		lua_pushcfunction(lua, lua_glTexImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTexImage2D");
		lua_pushcfunction(lua, lua_glCopyTexImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTexImage1D");
		lua_pushcfunction(lua, lua_glCopyTexImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexSubImage3D");
		lua_pushcfunction(lua, lua_glTexSubImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexSubImage2D");
		lua_pushcfunction(lua, lua_glTexSubImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexSubImage1D");
		lua_pushcfunction(lua, lua_glTexSubImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTexSubImage3D");
		lua_pushcfunction(lua, lua_glCopyTexSubImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTexSubImage2D");
		lua_pushcfunction(lua, lua_glCopyTexSubImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTexSubImage1D");
		lua_pushcfunction(lua, lua_glCopyTexSubImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureSubImage3D");
		lua_pushcfunction(lua, lua_glTextureSubImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureSubImage2D");
		lua_pushcfunction(lua, lua_glTextureSubImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureSubImage1D");
		lua_pushcfunction(lua, lua_glTextureSubImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTextureSubImage3D");
		lua_pushcfunction(lua, lua_glCopyTextureSubImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTextureSubImage2D");
		lua_pushcfunction(lua, lua_glCopyTextureSubImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyTextureSubImage1D");
		lua_pushcfunction(lua, lua_glCopyTextureSubImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompressedTexImage3D");
		lua_pushcfunction(lua, lua_glCompressedTexImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompressedTexImage2D");
		lua_pushcfunction(lua, lua_glCompressedTexImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompressedTexImage1D");
		lua_pushcfunction(lua, lua_glCompressedTexImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompressedTexSubImage3D");
		lua_pushcfunction(lua, lua_glCompressedTexSubImage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompressedTexSubImage2D");
		lua_pushcfunction(lua, lua_glCompressedTexSubImage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CompressedTexSubImage1D");
		lua_pushcfunction(lua, lua_glCompressedTexSubImage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexImage3DMultisample");
		lua_pushcfunction(lua, lua_glTexImage3DMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexImage2DMultisample");
		lua_pushcfunction(lua, lua_glTexImage2DMultisample);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "TexBufferRange");
		// lua_pushcfunction(lua, lua_glTexBufferRange);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "TextureBufferRange");
		lua_pushcfunction(lua, lua_glTextureBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexBuffer");
		lua_pushcfunction(lua, lua_glTexBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureBuffer");
		lua_pushcfunction(lua, lua_glTextureBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexParameteri");
		lua_pushcfunction(lua, lua_glTexParameteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexParameterf");
		lua_pushcfunction(lua, lua_glTexParameterf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexParameteriv");
		lua_pushcfunction(lua, lua_glTexParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexParameterfv");
		lua_pushcfunction(lua, lua_glTexParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexParameterIiv");
		lua_pushcfunction(lua, lua_glTexParameterIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexParamenterIuiv");
		lua_pushcfunction(lua, lua_glTexParamenterIuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureParamenteri");
		lua_pushcfunction(lua, lua_glTextureParamenteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureParamenterf");
		lua_pushcfunction(lua, lua_glTextureParamenterf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureParameteriv");
		lua_pushcfunction(lua, lua_glTextureParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureParameterfv");
		lua_pushcfunction(lua, lua_glTextureParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureParameterIiv");
		lua_pushcfunction(lua, lua_glTextureParameterIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureParameterIuiv");
		lua_pushcfunction(lua, lua_glTextureParameterIuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTexParameteriv");
		lua_pushcfunction(lua, lua_glGetTexParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTexParameterfv");
		lua_pushcfunction(lua, lua_glGetTexParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTexParameterIiv");
		lua_pushcfunction(lua, lua_glGetTexParameterIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTexParameterIuiv");
		lua_pushcfunction(lua, lua_glGetTexParameterIuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTextureParameteriv");
		lua_pushcfunction(lua, lua_glGetTextureParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTextureParameterfv");
		lua_pushcfunction(lua, lua_glGetTextureParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTextureParameterIiv");
		lua_pushcfunction(lua, lua_glGetTextureParameterIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTextureParameterIuiv");
		lua_pushcfunction(lua, lua_glGetTextureParameterIuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexLevelParameteriv");
		lua_pushcfunction(lua, lua_glTexLevelParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexLevelParameterfv");
		lua_pushcfunction(lua, lua_glTexLevelParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureLevelParameteriv");
		lua_pushcfunction(lua, lua_glTextureLevelParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureLevelParameterfv");
		lua_pushcfunction(lua, lua_glTextureLevelParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTexImage");
		lua_pushcfunction(lua, lua_glGetTexImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTextureImage");
		lua_pushcfunction(lua, lua_glGetTextureImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetnTexImage");
		lua_pushcfunction(lua, lua_glGetnTexImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTextureSubImage");
		lua_pushcfunction(lua, lua_glGetTextureSubImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetCompressedTexImage");
		lua_pushcfunction(lua, lua_glGetCompressedTexImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetCompressedTextureImage");
		lua_pushcfunction(lua, lua_glGetCompressedTextureImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetnCompressedTexImage");
		lua_pushcfunction(lua, lua_glGetnCompressedTexImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetCompressedTextureSubImage");
		lua_pushcfunction(lua, lua_glGetCompressedTextureSubImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenerateMipmap");
		lua_pushcfunction(lua, lua_glGenerateMipmap);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenerateTextureMipmap");
		lua_pushcfunction(lua, lua_glGenerateTextureMipmap);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "TextureView");
		// lua_pushcfunction(lua, lua_glTextureView);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "TexStorate1D");
		// lua_pushcfunction(lua, lua_glTexStorate1D);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "TexStorage2D");
		// lua_pushcfunction(lua, lua_glTexStorage2D);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "TexStorage3D");
		// lua_pushcfunction(lua, lua_glTexStorage3D);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "TextureStorage1D");
		lua_pushcfunction(lua, lua_glTextureStorage1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureStorage2D");
		lua_pushcfunction(lua, lua_glTextureStorage2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureStorage3D");
		lua_pushcfunction(lua, lua_glTextureStorage3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexStorage2DMultisample");
		lua_pushcfunction(lua, lua_glTexStorage2DMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TexStorage3DMultisample");
		lua_pushcfunction(lua, lua_glTexStorage3DMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureStorage2DMultisample");
		lua_pushcfunction(lua, lua_glTextureStorage2DMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureStorage3DMultisample");
		lua_pushcfunction(lua, lua_glTextureStorage3DMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidadeTexSubImage");
		lua_pushcfunction(lua, lua_glInvalidadeTexSubImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidadeTexImage");
		lua_pushcfunction(lua, lua_glInvalidadeTexImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearTexSubImage");
		lua_pushcfunction(lua, lua_glClearTexSubImage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearTexImage");
		lua_pushcfunction(lua, lua_glClearTexImage);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "BindImageTexture");
		// lua_pushcfunction(lua, lua_glBindImageTexture);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "BindImageTextures");
		// lua_pushcfunction(lua, lua_glBindImageTextures);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "BindFramebuffer");
		lua_pushcfunction(lua, lua_glBindFramebuffer);
		lua_settable(lua, -3);
//		lua_pushstring(lua, "CreateFramebuffer");
//		lua_pushcfunction(lua, lua_glCreateFramebuffer);
//		lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateFramebuffers");
		// lua_pushcfunction(lua, lua_glCreateFramebuffers);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "GenFramebuffer");
		lua_pushcfunction(lua, lua_glGenFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenFramebuffers");
		lua_pushcfunction(lua, lua_glGenFramebuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteFramebuffer");
		lua_pushcfunction(lua, lua_glDeleteFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteFramebuffers");
		lua_pushcfunction(lua, lua_glDeleteFramebuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsFramebuffer");
		lua_pushcfunction(lua, lua_glIsFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferParameteri");
		lua_pushcfunction(lua, lua_glFramebufferParameteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferParameteri");
		lua_pushcfunction(lua, lua_glNamedFramebufferParameteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetFramebufferParameteriv");
		lua_pushcfunction(lua, lua_glGetFramebufferParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetNamedFramebufferParameteriv");
		lua_pushcfunction(lua, lua_glGetNamedFramebufferParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetFramebufferAttachmentParameteriv");
		lua_pushcfunction(lua, lua_glGetFramebufferAttachmentParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetNamedFramebufferAttachmentParameteriv");
		lua_pushcfunction(lua, lua_glGetNamedFramebufferAttachmentParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindRenderbuffer");
		lua_pushcfunction(lua, lua_glBindRenderbuffer);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateRenderbuffer");
		// lua_pushcfunction(lua, lua_glCreateRenderbuffer);
		// lua_settable(lua, -3);
		// lua_pushstring(lua, "CreateRenderbuffers");
		// lua_pushcfunction(lua, lua_glCreateRenderbuffers);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "GenRenderbuffer");
		lua_pushcfunction(lua, lua_glGenRenderbuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenRenderbuffers");
		lua_pushcfunction(lua, lua_glGenRenderbuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteRenderbuffer");
		lua_pushcfunction(lua, lua_glDeleteRenderbuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteRenderbuffers");
		lua_pushcfunction(lua, lua_glDeleteRenderbuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsRenderbuffer");
		lua_pushcfunction(lua, lua_glIsRenderbuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RenderbufferStorageMultisample");
		lua_pushcfunction(lua, lua_glRenderbufferStorageMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedRenderbufferStorageMultisample");
		lua_pushcfunction(lua, lua_glNamedRenderbufferStorageMultisample);
		lua_settable(lua, -3);
		lua_pushstring(lua, "RenderbufferStorage");
		lua_pushcfunction(lua, lua_glRenderbufferStorage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedRenderbufferStorage");
		lua_pushcfunction(lua, lua_glNamedRenderbufferStorage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetRenderbufferParameteriv");
		lua_pushcfunction(lua, lua_glGetRenderbufferParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetNamedRenderbufferParameteriv");
		lua_pushcfunction(lua, lua_glGetNamedRenderbufferParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferRenderbuffer");
		lua_pushcfunction(lua, lua_glFramebufferRenderbuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferRenderbuffer");
		lua_pushcfunction(lua, lua_glNamedFramebufferRenderbuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferTexture");
		lua_pushcfunction(lua, lua_glFramebufferTexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferTexture");
		lua_pushcfunction(lua, lua_glNamedFramebufferTexture);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferTexture1D");
		lua_pushcfunction(lua, lua_glFramebufferTexture1D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferTexture2D");
		lua_pushcfunction(lua, lua_glFramebufferTexture2D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferTexture3D");
		lua_pushcfunction(lua, lua_glFramebufferTexture3D);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FramebufferTextureLayer");
		lua_pushcfunction(lua, lua_glFramebufferTextureLayer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferTextureLayer");
		lua_pushcfunction(lua, lua_glNamedFramebufferTextureLayer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TextureBarrier");
		lua_pushcfunction(lua, lua_glTextureBarrier);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CheckFramebufferStatus");
		lua_pushcfunction(lua, lua_glCheckFramebufferStatus);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CheckNamedFramebufferStatus");
		lua_pushcfunction(lua, lua_glCheckNamedFramebufferStatus);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PatchParameteri");
		lua_pushcfunction(lua, lua_glPatchParameteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib1s");
		lua_pushcfunction(lua, lua_glVertexAttrib1s);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib2s");
		lua_pushcfunction(lua, lua_glVertexAttrib2s);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib3s");
		lua_pushcfunction(lua, lua_glVertexAttrib3s);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4s");
		lua_pushcfunction(lua, lua_glVertexAttrib4s);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib1f");
		lua_pushcfunction(lua, lua_glVertexAttrib1f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib2f");
		lua_pushcfunction(lua, lua_glVertexAttrib2f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib3f");
		lua_pushcfunction(lua, lua_glVertexAttrib3f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4f");
		lua_pushcfunction(lua, lua_glVertexAttrib4f);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib1d");
		lua_pushcfunction(lua, lua_glVertexAttrib1d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib2d");
		lua_pushcfunction(lua, lua_glVertexAttrib2d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib3d");
		lua_pushcfunction(lua, lua_glVertexAttrib3d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4d");
		lua_pushcfunction(lua, lua_glVertexAttrib4d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib1sv");
		lua_pushcfunction(lua, lua_glVertexAttrib1sv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib2sv");
		lua_pushcfunction(lua, lua_glVertexAttrib2sv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib3sv");
		lua_pushcfunction(lua, lua_glVertexAttrib3sv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib1fv");
		lua_pushcfunction(lua, lua_glVertexAttrib1fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib2fv");
		lua_pushcfunction(lua, lua_glVertexAttrib2fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib3fv");
		lua_pushcfunction(lua, lua_glVertexAttrib3fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib1dv");
		lua_pushcfunction(lua, lua_glVertexAttrib1dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib2dv");
		lua_pushcfunction(lua, lua_glVertexAttrib2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib3dv");
		lua_pushcfunction(lua, lua_glVertexAttrib3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4bv");
		lua_pushcfunction(lua, lua_glVertexAttrib4bv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4sv");
		lua_pushcfunction(lua, lua_glVertexAttrib4sv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4iv");
		lua_pushcfunction(lua, lua_glVertexAttrib4iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4fv");
		lua_pushcfunction(lua, lua_glVertexAttrib4fv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4dv");
		lua_pushcfunction(lua, lua_glVertexAttrib4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4ubv");
		lua_pushcfunction(lua, lua_glVertexAttrib4ubv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4usv");
		lua_pushcfunction(lua, lua_glVertexAttrib4usv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4uiv");
		lua_pushcfunction(lua, lua_glVertexAttrib4uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nub");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nub);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nb");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nb);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nbv");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nbv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nsv");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nsv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Niv");
		lua_pushcfunction(lua, lua_glVertexAttrib4Niv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nubv");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nubv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nusv");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nusv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttrib4Nuiv");
		lua_pushcfunction(lua, lua_glVertexAttrib4Nuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI1i");
		lua_pushcfunction(lua, lua_glVertexAttribI1i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI2i");
		lua_pushcfunction(lua, lua_glVertexAttribI2i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI3i");
		lua_pushcfunction(lua, lua_glVertexAttribI3i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4i");
		lua_pushcfunction(lua, lua_glVertexAttribI4i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI1ui");
		lua_pushcfunction(lua, lua_glVertexAttribI1ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI2ui");
		lua_pushcfunction(lua, lua_glVertexAttribI2ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI3ui");
		lua_pushcfunction(lua, lua_glVertexAttribI3ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4ui");
		lua_pushcfunction(lua, lua_glVertexAttribI4ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI1uiv");
		lua_pushcfunction(lua, lua_glVertexAttribI1uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI2uiv");
		lua_pushcfunction(lua, lua_glVertexAttribI2uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI3uiv");
		lua_pushcfunction(lua, lua_glVertexAttribI3uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4uiv");
		lua_pushcfunction(lua, lua_glVertexAttribI4uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4bv");
		lua_pushcfunction(lua, lua_glVertexAttribI4bv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4sv");
		lua_pushcfunction(lua, lua_glVertexAttribI4sv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4ubv");
		lua_pushcfunction(lua, lua_glVertexAttribI4ubv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribI4usv");
		lua_pushcfunction(lua, lua_glVertexAttribI4usv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL1d");
		lua_pushcfunction(lua, lua_glVertexAttribL1d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL2d");
		lua_pushcfunction(lua, lua_glVertexAttribL2d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL3d");
		lua_pushcfunction(lua, lua_glVertexAttribL3d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL4d");
		lua_pushcfunction(lua, lua_glVertexAttribL4d);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL1dv");
		lua_pushcfunction(lua, lua_glVertexAttribL1dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL2dv");
		lua_pushcfunction(lua, lua_glVertexAttribL2dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL3dv");
		lua_pushcfunction(lua, lua_glVertexAttribL3dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribL4dv");
		lua_pushcfunction(lua, lua_glVertexAttribL4dv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP1ui");
		lua_pushcfunction(lua, lua_glVertexAttribP1ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP2ui");
		lua_pushcfunction(lua, lua_glVertexAttribP2ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP3ui");
		lua_pushcfunction(lua, lua_glVertexAttribP3ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP4ui");
		lua_pushcfunction(lua, lua_glVertexAttribP4ui);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP1uiv");
		lua_pushcfunction(lua, lua_glVertexAttribP1uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP2uiv");
		lua_pushcfunction(lua, lua_glVertexAttribP2uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP3uiv");
		lua_pushcfunction(lua, lua_glVertexAttribP3uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribP4uiv");
		lua_pushcfunction(lua, lua_glVertexAttribP4uiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenVertexArray");
		lua_pushcfunction(lua, lua_glGenVertexArray);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenVertexArrays");
		lua_pushcfunction(lua, lua_glGenVertexArrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteVertexArrays");
		lua_pushcfunction(lua, lua_glDeleteVertexArrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindVertexArray");
		lua_pushcfunction(lua, lua_glBindVertexArray);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CreateVertexArrays");
		lua_pushcfunction(lua, lua_glCreateVertexArrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "IsVertexArray");
		lua_pushcfunction(lua, lua_glIsVertexArray);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayElementBuffer");
		lua_pushcfunction(lua, lua_glVertexArrayElementBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribFormat");
		lua_pushcfunction(lua, lua_glVertexAttribFormat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribIFormat");
		lua_pushcfunction(lua, lua_glVertexAttribIFormat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribLFormat");
		lua_pushcfunction(lua, lua_glVertexAttribLFormat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayAttribFormat");
		lua_pushcfunction(lua, lua_glVertexArrayAttribFormat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayAttribIFormat");
		lua_pushcfunction(lua, lua_glVertexArrayAttribIFormat);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayAttribLFormat");
		lua_pushcfunction(lua, lua_glVertexArrayAttribLFormat);
		lua_settable(lua, -3);
		// lua_pushstring(lua, "BindVertexBuffer");
		// lua_pushcfunction(lua, lua_glBindVertexBuffer);
		// lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayVertexBuffer");
		lua_pushcfunction(lua, lua_glVertexArrayVertexBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindVertexBuffers");
		lua_pushcfunction(lua, lua_glBindVertexBuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayVertexBuffers");
		lua_pushcfunction(lua, lua_glVertexArrayVertexBuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribBinding");
		lua_pushcfunction(lua, lua_glVertexAttribBinding);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayAttribBinding");
		lua_pushcfunction(lua, lua_glVertexArrayAttribBinding);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribPointer");
		lua_pushcfunction(lua, lua_glVertexAttribPointer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribLPointer");
		lua_pushcfunction(lua, lua_glVertexAttribLPointer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EnableVertexAttribArray");
		lua_pushcfunction(lua, lua_glEnableVertexAttribArray);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EnableVertexArrayAttrib");
		lua_pushcfunction(lua, lua_glEnableVertexArrayAttrib);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DisableVertexAttribArray");
		lua_pushcfunction(lua, lua_glDisableVertexAttribArray);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DisableVertexArrayAttrib");
		lua_pushcfunction(lua, lua_glDisableVertexArrayAttrib);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexBindingDivisor");
		lua_pushcfunction(lua, lua_glVertexBindingDivisor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexArrayBindingDivisor");
		lua_pushcfunction(lua, lua_glVertexArrayBindingDivisor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "VertexAttribDivisor");
		lua_pushcfunction(lua, lua_glVertexAttribDivisor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PrimitiveRestartIndex");
		lua_pushcfunction(lua, lua_glPrimitiveRestartIndex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawArrays");
		lua_pushcfunction(lua, lua_glDrawArrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawArraysInstancedBasedInstance");
		lua_pushcfunction(lua, lua_glDrawArraysInstancedBasedInstance);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawArraysInstanced");
		lua_pushcfunction(lua, lua_glDrawArraysInstanced);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawArraysIndirect");
		lua_pushcfunction(lua, lua_glDrawArraysIndirect);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MultiDrawArrays");
		lua_pushcfunction(lua, lua_glMultiDrawArrays);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MultiDrawArraysIndirect");
		lua_pushcfunction(lua, lua_glMultiDrawArraysIndirect);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElements");
		lua_pushcfunction(lua, lua_glDrawElements);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElementsInstancedBaseInstance");
		lua_pushcfunction(lua, lua_glDrawElementsInstancedBaseInstance);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElementInstanced");
		lua_pushcfunction(lua, lua_glDrawElementInstanced);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MultiDrawElements");
		lua_pushcfunction(lua, lua_glMultiDrawElements);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawRangeElements");
		lua_pushcfunction(lua, lua_glDrawRangeElements);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElementsBaseVertex");
		lua_pushcfunction(lua, lua_glDrawElementsBaseVertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawRangeElementsBaseVertex");
		lua_pushcfunction(lua, lua_glDrawRangeElementsBaseVertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElementsInstancedBaseVertex");
		lua_pushcfunction(lua, lua_glDrawElementsInstancedBaseVertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElementsInstancedBaseVertexBaseInstance");
		lua_pushcfunction(lua, lua_glDrawElementsInstancedBaseVertexBaseInstance);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawElementsIndirect");
		lua_pushcfunction(lua, lua_glDrawElementsIndirect);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MultiDrawElementsIndirect");
		lua_pushcfunction(lua, lua_glMultiDrawElementsIndirect);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MultiDrawElementsBaseVertex");
		lua_pushcfunction(lua, lua_glMultiDrawElementsBaseVertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexArrayiv");
		lua_pushcfunction(lua, lua_glGetVertexArrayiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexArrayIndexdiv");
		lua_pushcfunction(lua, lua_glGetVertexArrayIndexdiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexArrayIndexd64iv");
		lua_pushcfunction(lua, lua_glGetVertexArrayIndexd64iv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribdv");
		lua_pushcfunction(lua, lua_glGetVertexAttribdv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribfv");
		lua_pushcfunction(lua, lua_glGetVertexAttribfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribiv");
		lua_pushcfunction(lua, lua_glGetVertexAttribiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribIiv");
		lua_pushcfunction(lua, lua_glGetVertexAttribIiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribIuiv");
		lua_pushcfunction(lua, lua_glGetVertexAttribIuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribLdv");
		lua_pushcfunction(lua, lua_glGetVertexAttribLdv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetVertexAttribPointerv");
		lua_pushcfunction(lua, lua_glGetVertexAttribPointerv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BeginConditionalRender");
		lua_pushcfunction(lua, lua_glBeginConditionalRender);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EndConditionalRender");
		lua_pushcfunction(lua, lua_glEndConditionalRender);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindAttribLocation");
		lua_pushcfunction(lua, lua_glBindAttribLocation);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetActiveAttrib");
		lua_pushcfunction(lua, lua_glGetActiveAttrib);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetAttribLocation");
		lua_pushcfunction(lua, lua_glGetAttribLocation);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TransformFeedbackVaryings");
		lua_pushcfunction(lua, lua_glTransformFeedbackVaryings);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTransformFeedbackVarying");
		lua_pushcfunction(lua, lua_glGetTransformFeedbackVarying);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ValidateProgram");
		lua_pushcfunction(lua, lua_glValidateProgram);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ValidadteProgramPipeline");
		lua_pushcfunction(lua, lua_glValidadteProgramPipeline);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PatchParameterfv");
		lua_pushcfunction(lua, lua_glPatchParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GenTransformFeedbacks");
		lua_pushcfunction(lua, lua_glGenTransformFeedbacks);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DeleteTransformFeedbacks");
		lua_pushcfunction(lua, lua_glDeleteTransformFeedbacks);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BindTransformFeedback");
		lua_pushcfunction(lua, lua_glBindTransformFeedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CreateTransformFeedbacks");
		lua_pushcfunction(lua, lua_glCreateTransformFeedbacks);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BeginTransformFeedback");
		lua_pushcfunction(lua, lua_glBeginTransformFeedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EndTransformFeedback");
		lua_pushcfunction(lua, lua_glEndTransformFeedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PauseTransformFeedback");
		lua_pushcfunction(lua, lua_glPauseTransformFeedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ResumeTransformFeedback");
		lua_pushcfunction(lua, lua_glResumeTransformFeedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TransformFeedbackBufferRange");
		lua_pushcfunction(lua, lua_glTransformFeedbackBufferRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "TransformFeedbackBufferBase");
		lua_pushcfunction(lua, lua_glTransformFeedbackBufferBase);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawTransformFeedback");
		lua_pushcfunction(lua, lua_glDrawTransformFeedback);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawTransformFeedbackInstanced");
		lua_pushcfunction(lua, lua_glDrawTransformFeedbackInstanced);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawTransformFeedbackStream");
		lua_pushcfunction(lua, lua_glDrawTransformFeedbackStream);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawTransformFeedbackStreamInstanced");
		lua_pushcfunction(lua, lua_glDrawTransformFeedbackStreamInstanced);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ProvokingVertex");
		lua_pushcfunction(lua, lua_glProvokingVertex);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClipControl");
		lua_pushcfunction(lua, lua_glClipControl);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DepthRangeArrayv");
		lua_pushcfunction(lua, lua_glDepthRangeArrayv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DepthRangeIndexed");
		lua_pushcfunction(lua, lua_glDepthRangeIndexed);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DepthRange");
		lua_pushcfunction(lua, lua_glDepthRange);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DepthRangef");
		lua_pushcfunction(lua, lua_glDepthRangef);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ViewportArrayv");
		lua_pushcfunction(lua, lua_glViewportArrayv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ViewportIndexedf");
		lua_pushcfunction(lua, lua_glViewportIndexedf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ViewportIndexedfv");
		lua_pushcfunction(lua, lua_glViewportIndexedfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Viewport");
		lua_pushcfunction(lua, lua_glViewport);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetMultisamplefv");
		lua_pushcfunction(lua, lua_glGetMultisamplefv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "MinSampleShading");
		lua_pushcfunction(lua, lua_glMinSampleShading);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PointSize");
		lua_pushcfunction(lua, lua_glPointSize);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PointParameteri");
		lua_pushcfunction(lua, lua_glPointParameteri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PointParameterf");
		lua_pushcfunction(lua, lua_glPointParameterf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PointParameteriv");
		lua_pushcfunction(lua, lua_glPointParameteriv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PointParameterfv");
		lua_pushcfunction(lua, lua_glPointParameterfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LineWidth");
		lua_pushcfunction(lua, lua_glLineWidth);
		lua_settable(lua, -3);
		lua_pushstring(lua, "FrontFace");
		lua_pushcfunction(lua, lua_glFrontFace);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PolygonMode");
		lua_pushcfunction(lua, lua_glPolygonMode);
		lua_settable(lua, -3);
		lua_pushstring(lua, "PolygonOffset");
		lua_pushcfunction(lua, lua_glPolygonOffset);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ScissorArrayv");
		lua_pushcfunction(lua, lua_glScissorArrayv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ScissorIndexed");
		lua_pushcfunction(lua, lua_glScissorIndexed);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ScissorIndexedv");
		lua_pushcfunction(lua, lua_glScissorIndexedv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Scissor");
		lua_pushcfunction(lua, lua_glScissor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SampleCoverage");
		lua_pushcfunction(lua, lua_glSampleCoverage);
		lua_settable(lua, -3);
		lua_pushstring(lua, "SampleMaski");
		lua_pushcfunction(lua, lua_glSampleMaski);
		lua_settable(lua, -3);
		lua_pushstring(lua, "StencilFunc");
		lua_pushcfunction(lua, lua_glStencilFunc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "StencilFuncSeparate");
		lua_pushcfunction(lua, lua_glStencilFuncSeparate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "StencilOp");
		lua_pushcfunction(lua, lua_glStencilOp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "StencilOpSeparate");
		lua_pushcfunction(lua, lua_glStencilOpSeparate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DepthFunc");
		lua_pushcfunction(lua, lua_glDepthFunc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BeginQuery");
		lua_pushcfunction(lua, lua_glBeginQuery);
		lua_settable(lua, -3);
		lua_pushstring(lua, "EndQuery");
		lua_pushcfunction(lua, lua_glEndQuery);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendEquation");
		lua_pushcfunction(lua, lua_glBlendEquation);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendEquationSeparate");
		lua_pushcfunction(lua, lua_glBlendEquationSeparate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendEquationi");
		lua_pushcfunction(lua, lua_glBlendEquationi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendEquationSeparatei");
		lua_pushcfunction(lua, lua_glBlendEquationSeparatei);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendFunc");
		lua_pushcfunction(lua, lua_glBlendFunc);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendFuncSeparate");
		lua_pushcfunction(lua, lua_glBlendFuncSeparate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendFunci");
		lua_pushcfunction(lua, lua_glBlendFunci);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendFuncSeparatei");
		lua_pushcfunction(lua, lua_glBlendFuncSeparatei);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlendColor");
		lua_pushcfunction(lua, lua_glBlendColor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "LogicOp");
		lua_pushcfunction(lua, lua_glLogicOp);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Hint");
		lua_pushcfunction(lua, lua_glHint);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawBuffer");
		lua_pushcfunction(lua, lua_glDrawBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferDrawBuffer");
		lua_pushcfunction(lua, lua_glNamedFramebufferDrawBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DrawBuffers");
		lua_pushcfunction(lua, lua_glDrawBuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferDrawBuffers");
		lua_pushcfunction(lua, lua_glNamedFramebufferDrawBuffers);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ColorMask");
		lua_pushcfunction(lua, lua_glColorMask);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ColorMaski");
		lua_pushcfunction(lua, lua_glColorMaski);
		lua_settable(lua, -3);
		lua_pushstring(lua, "DepthMask");
		lua_pushcfunction(lua, lua_glDepthMask);
		lua_settable(lua, -3);
		lua_pushstring(lua, "StencilMask");
		lua_pushcfunction(lua, lua_glStencilMask);
		lua_settable(lua, -3);
		lua_pushstring(lua, "StencilMaskSeparate");
		lua_pushcfunction(lua, lua_glStencilMaskSeparate);
		lua_settable(lua, -3);
		lua_pushstring(lua, "Clear");
		lua_pushcfunction(lua, lua_glClear);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearColor");
		lua_pushcfunction(lua, lua_glClearColor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearDepth");
		lua_pushcfunction(lua, lua_glClearDepth);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearDepthf");
		lua_pushcfunction(lua, lua_glClearDepthf);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearStencil");
		lua_pushcfunction(lua, lua_glClearStencil);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearBufferiv");
		lua_pushcfunction(lua, lua_glClearBufferiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearBufferfv");
		lua_pushcfunction(lua, lua_glClearBufferfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearBufferuiv");
		lua_pushcfunction(lua, lua_glClearBufferuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearNamedFramebufferiv");
		lua_pushcfunction(lua, lua_glClearNamedFramebufferiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearNamedFramebufferfv");
		lua_pushcfunction(lua, lua_glClearNamedFramebufferfv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearNamedFramebufferuiv");
		lua_pushcfunction(lua, lua_glClearNamedFramebufferuiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearBufferfi");
		lua_pushcfunction(lua, lua_glClearBufferfi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClearNamedFramebufferfi");
		lua_pushcfunction(lua, lua_glClearNamedFramebufferfi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidateSubFramebuffer");
		lua_pushcfunction(lua, lua_glInvalidateSubFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidateNamedFramebufferSubData");
		lua_pushcfunction(lua, lua_glInvalidateNamedFramebufferSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidateFramebuffer");
		lua_pushcfunction(lua, lua_glInvalidateFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "InvalidateNamedFramebufferData");
		lua_pushcfunction(lua, lua_glInvalidateNamedFramebufferData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ReadBuffer");
		lua_pushcfunction(lua, lua_glReadBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "NamedFramebufferReadBuffer");
		lua_pushcfunction(lua, lua_glNamedFramebufferReadBuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ReadPixels");
		lua_pushcfunction(lua, lua_glReadPixels);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ReadnPixels");
		lua_pushcfunction(lua, lua_glReadnPixels);
		lua_settable(lua, -3);
		lua_pushstring(lua, "ClampColor");
		lua_pushcfunction(lua, lua_glClampColor);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlitFramebuffer");
		lua_pushcfunction(lua, lua_glBlitFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "BlitNamedFramebuffer");
		lua_pushcfunction(lua, lua_glBlitNamedFramebuffer);
		lua_settable(lua, -3);
		lua_pushstring(lua, "CopyImageSubData");
		lua_pushcfunction(lua, lua_glCopyImageSubData);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetBooleanv");
		lua_pushcfunction(lua, lua_glGetBooleanv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetIntegerv");
		lua_pushcfunction(lua, lua_glGetIntegerv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetInteger64v");
		lua_pushcfunction(lua, lua_glGetInteger64v);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetFloatv");
		lua_pushcfunction(lua, lua_glGetFloatv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetDoublev");
		lua_pushcfunction(lua, lua_glGetDoublev);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetDoublei");
		lua_pushcfunction(lua, lua_glGetDoublei);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetBooleani");
		lua_pushcfunction(lua, lua_glGetBooleani);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetIntegeri");
		lua_pushcfunction(lua, lua_glGetIntegeri);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetFloati");
		lua_pushcfunction(lua, lua_glGetFloati);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetInteger64i");
		lua_pushcfunction(lua, lua_glGetInteger64i);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetPointerv");
		lua_pushcfunction(lua, lua_glGetPointerv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetString");
		lua_pushcfunction(lua, lua_glGetString);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetStringi");
		lua_pushcfunction(lua, lua_glGetStringi);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetInternalformativ");
		lua_pushcfunction(lua, lua_glGetInternalformativ);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetInternalformati64v");
		lua_pushcfunction(lua, lua_glGetInternalformati64v);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTransformFeedbackiv");
		lua_pushcfunction(lua, lua_glGetTransformFeedbackiv);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTransformFeedbacki");
		lua_pushcfunction(lua, lua_glGetTransformFeedbacki);
		lua_settable(lua, -3);
		lua_pushstring(lua, "GetTransformFeedbacki64");
		lua_pushcfunction(lua, lua_glGetTransformFeedbacki64);
		lua_settable(lua, -3);

	}
	lua_pop(lua, 1);
	return 1;
}

// End of file.
