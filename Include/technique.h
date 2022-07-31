/*

        Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <list>
#include <GL/glew.h>
/*
  OpenGL 加载库是一个在运行时加载指向 OpenGL 函数、核心以及扩展的指针的库
  扩展加载库还抽象出不同平台上加载机制之间的差异。
  大多数扩展加载库根本不需要包含 gl.h。 相反，它们提供了自己必须使用的header 。
  GLEW  OpenGL Extension Wrangler
			与大多数其他加载程序一样，不应在 glew.h 之前包含 gl.h、glext.h 或任何其他与 gl 相关的头文件
		    使用 glew.h 替换  gl.h
  GL3W   OpenGL 3 and 4  
  glad (Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator)


  LearnOpenGL 使用 glfw  + glad     实现窗口(窗口创建)+gl函数指针(库加载)
  Ogldev          使用 GLUT + GLEW  实现窗口(窗口创建)+gl函数指针(库加载)

*/

class Technique
{
public:

    Technique();

    virtual ~Technique();

    virtual bool Init();

    void Enable();

    GLuint GetProgram() const { return m_shaderProg; }

protected:

    bool AddShader(GLenum ShaderType, const char* pFilename);

    bool Finalize();

    GLint GetUniformLocation(const char* pUniformName);

    GLuint m_shaderProg = 0;

private:

    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
};

#endif  /* TECHNIQUE_H */
