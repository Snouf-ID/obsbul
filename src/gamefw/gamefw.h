
#ifndef GAMEFW_H
#define GAMEFW_H

#include "../common.h"

#define GL3_PROTOTYPES
#include <GL3/gl3.h>


/**
 * @brief Exception thrown by checkOpenGLError when there's an OpenGL internal error.
 */
class OpenGLError: public exception {};

/**
 * Checks if OpenGL has generated an error.
 * 
 * @throw OpenGLError When an error has occured.
 */
inline void checkOpenGLError()
{
    int error;
    if((error = glGetError()) != GL_NO_ERROR)
    {
        string s;
        switch(error)
        {
        case GL_INVALID_ENUM:
            s = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            s = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            s = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            s = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            s = "GL_OUT_OF_MEMORY";
            break;
        default:
            s = "UNKNOWN";
            break;
        }
        DLOG(ERROR) << "OpenGL Error: " << s.c_str();
        throw OpenGLError();
    }
}

#include <bullet/btBulletDynamicsCommon.h>

#include "game.h"
#include "gamestate.h"
#include "rendercontext.h"
#include "shaderfactory.h"
#include "shaderprogram.h"
#include "renderjob.h"
#include "entity.h"
#include "entityfactory.h"
#include "fileservice.h"
#include "locator.h"

#endif
