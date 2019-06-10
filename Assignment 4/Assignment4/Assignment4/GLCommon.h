/******************************************************************************
                         .88888.   888888ba  dP     dP 
                        d8'   `88  88    `8b 88     88 
                        88        a88aaaa8P' 88     88 
                        88   YP88  88        88     88 
                        Y8.   .88  88        Y8.   .8P 
                         `88888'   dP        `Y88888P' 
                                                       
                                                       
   a88888b.                                         dP   oo                   
  d8'   `88                                         88                        
  88        .d8888b. 88d8b.d8b. 88d888b. dP    dP d8888P dP 88d888b. .d8888b. 
  88        88'  `88 88'`88'`88 88'  `88 88    88   88   88 88'  `88 88'  `88 
  Y8.   .88 88.  .88 88  88  88 88.  .88 88.  .88   88   88 88    88 88.  .88 
   Y88888P' `88888P' dP  dP  dP 88Y888P' `88888P'   dP   dP dP    dP `8888P88 
                                88                                        .88 
                                dP                                    d8888P  
******************************************************************************/

#ifndef GL_COMMON_H
#define GL_COMMON_H

// for using OpenGL
#include "GL/glew.h"
#include <GLFW/glfw3.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//OpenGL utility functions

bool CreateShaderFromFile(const char* Path, GLhandleARB shader);

//utility function which attempts to compile a GLSL shader, then prints the error messages on failure
bool CompileGLSLShader(GLhandleARB shader);

bool LinkGLSLProgram(GLhandleARB program);

#define SAFE_RELEASE_GL_BUFFER(obj)  do {if(obj){ glDeleteBuffers(1, &obj); obj = 0; }} while(0)
#define SAFE_RELEASE_GL_SHADER(obj)  do {if(obj){ glDeleteShader(obj); obj = 0; }} while(0)
#define SAFE_RELEASE_GL_PROGRAM(obj) do {if(obj){ glDeleteProgram(obj); obj = 0; }} while(0)

#define CHECK_FOR_OGL_ERROR()                                  \
   do {                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
     if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
     }                                                         \
   } while(0)

#define V_RETURN_OGL_ERROR()                            \
	do {                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
	if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
       return false;                                           \
     }                                                         \
   } while(0)


#endif // GL_COMMON_H
