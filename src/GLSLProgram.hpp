/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: GLSLProgram.hpp
Purpose: Functions for compiling shaders and passing uniforms
Language: C++ Visual Studio 2015
Platform: Windows 7
Project: cs300_ivan.iglesias_1
Author: Ivan Iglesias, ivan.iglesias
Creation date: 5/21/2018
----------------------------------------------------------------------------------------------------------*/
#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <GL/gl3w.h>


#include <string>
using std::string;

#include <glm/glm.hpp>

namespace GLSLShader
{
    enum GLSLShaderType
    {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        TESS_CONTROL,
        TESS_EVALUATION
    };
};

/*
 * @brief   Class encapsulating a GLSL Program
 *
 * @detail  Class encapsulating a GLSL Program that also adds some member functions to set uniform variables. This class was mostly copied from the Cookbook code.
 */
class GLSLProgram
{
    public:

        static const std::string COLOR_TEX_PREFIX;
        static const std::string NORMAL_MAP_TEX_PREFIX;

		GLSLProgram();
		~GLSLProgram();

        bool   compileShaderFromFile( const char * fileName, GLSLShader::GLSLShaderType type );
        bool   compileShaderFromString( const string & source, GLSLShader::GLSLShaderType type );
        bool   link();
        bool   validate();
        void   use() const;

        string log() const;

        int    getHandle() const;
        bool   isLinked() const;

        void   bindAttribLocation( GLuint location, const char * name);
        void   bindFragDataLocation( GLuint location, const char * name );

        void   setUniform( const char *name, float x, float y, float z) const;
		void   setUniform( const char *name, float x, float y) const;
		void   setUniform( const char *name, const glm::vec2 & v) const;
        void   setUniform( const char *name, const glm::vec3 & v) const;
        void   setUniform( const char *name, const glm::vec4 & v) const;
        void   setUniform( const char *name, const glm::mat4 & m) const;
        void   setUniform( const char *name, const glm::mat3 & m) const;
        void   setUniform( const char *name, float val ) const;
        void   setUniform( const char *name, int val ) const;
        void   setUniform( const char *name, bool val ) const;

        void   printActiveUniforms() const;
        void   printActiveAttribs() const;

    private:
        int  getUniformLocation(const char * name ) const;
        bool fileExists( const string & fileName );

    private:
        int  handle_;
        bool linked_;
        string log_string_;
};

#endif // GLSLPROGRAM_H
