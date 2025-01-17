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


#include <stdio.h>

#include "ogldev_util.h"
#include "gbuffer.h"
#include "ogldev_texture.h"

GBuffer::GBuffer()
{
    m_fbo = 0;
        m_depthTexture = 0;
    ZERO_MEM(m_textures);
}

GBuffer::~GBuffer()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_textures[0] != 0) {
        glDeleteTextures(ARRAY_SIZE_IN_ELEMENTS(m_textures), m_textures);
    }

        if (m_depthTexture != 0) {
                glDeleteTextures(1, &m_depthTexture);
        }
}


bool GBuffer::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create the gbuffer textures
    glGenTextures(ARRAY_SIZE_IN_ELEMENTS(m_textures), m_textures);
    glGenTextures(1, &m_depthTexture);

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_textures) ; i++) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // depth
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	// 
	// 2.0是没有深度纹理的 3.0才有 
	// GL_DEPTH24_STENCIL8			GL_DEPTH_STENCIL			GL_UNSIGNED_INT_24_8
	// GL_DEPTH_COMPONENT32F	GL_DEPTH_COMPONENT		GL_FLOAT
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

	// MRT 多重渲染 
    GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,
											GL_COLOR_ATTACHMENT1,
											GL_COLOR_ATTACHMENT2,
											GL_COLOR_ATTACHMENT3 };


    glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);

	// 不同于 glDrawBuffer glReadBuffer
	// glDrawBuffer(GL_NONE) 设置到当前绑定的fbo, 没有颜色缓冲的fbo, 阴影 
	// glReadBuffer 指定读取的附件 glBlitFrameBuffer glReadPixelBuffer

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}


void GBuffer::BindForWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);// draw fbo指定为 m_fbo
}


void GBuffer::BindForReading()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo); // read fbo指定为 m_fbo 
}


void GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType)
{
	// 将特定纹理绑定到 GL_READ_BUFFER 目标（因为我们一次只能从单个纹理复制）
    glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType); 
}
