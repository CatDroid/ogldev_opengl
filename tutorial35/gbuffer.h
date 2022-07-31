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

#ifndef GBUFFER_H
#define	GBUFFER_H

#include <GL/glew.h>

#define GBUFFER_POSITION_TEXTURE_UNIT 0
#define GBUFFER_DIFFUSE_TEXTURE_UNIT  1
#define GBUFFER_NORMAL_TEXTURE_UNIT   2
#define GBUFFER_TEXCOORD_TEXTURE_UNIT 3

class GBuffer
{
public:

    enum GBUFFER_TEXTURE_TYPE // G 缓冲区所需的所有纹理
	{
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };
    
    GBuffer();

    ~GBuffer();

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

	// 在geometry pass几何处理阶段, 将纹理绑定为目标(输出目标)
    void BindForWriting();
   
	// 将 FBO 绑定为输入，因此可以将其内容转储到屏幕上
    void BindForReading();  
    
    void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

private:
                     
    GLuint m_fbo; // 一个fbo =  GBUFFER_NUM_TEXTURES个颜色附件  + 1个深度纹理
    GLuint m_textures[GBUFFER_NUM_TEXTURES];
    GLuint m_depthTexture;
};

#endif	/* SHADOWMAPFBO_H */

