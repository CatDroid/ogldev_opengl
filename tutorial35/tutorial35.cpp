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

    Tutorial 35 - Deferred Shading - Part 1
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ds_geom_pass_tech.h"
#include "ogldev_glut_backend.h"
#include "ogldev_basic_mesh.h"
#include "gbuffer.h"

#include "filesystem.h"

using namespace std;

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1024

class Tutorial35 : public ICallbacks, public OgldevApp
{
public:

    Tutorial35()
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;
    }

    ~Tutorial35()
    {
        SAFE_DELETE(m_pGameCamera);
    }

    bool Init()
    {
        if (!m_gbuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!m_DSGeomPassTech.Init()) {
            printf("Error initializing DSGeomPassTech\n");
            return false;
        }

                m_DSGeomPassTech.Enable();
                m_DSGeomPassTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

        if (!m_mesh.LoadMesh(FileSystem::getPath("Content/phoenix_ugv.md2")) ){
                        return false;
                }

#ifndef WIN32
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif

        return true;
    }

    void Run()
    {
        GLUTBackendRun(this); // this是 ICallbacks glut会间接从GLUTBackend回调过来这里
    }


    virtual void RenderSceneCB()
    {
        CalcFPS();

        m_scale += 0.05f;

        m_pGameCamera->OnRender();

        DSGeometryPass(); // 几何处理阶段  G缓冲区将包含最近像素的属性(the closest pixels)
		DSLightPass();		 // 光照计算阶段

        RenderFPS();

        glutSwapBuffers();
    }


    void DSGeometryPass()
    {
		m_DSGeomPassTech.Enable(); // 使用几何阶段的shader

        m_gbuffer.BindForWriting(); // G-Buffer 绑定 准备渲染到G-buffer

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Pipeline p;							 // 设置物体的旋转缩放 和 相机九参数 
        p.Scale(0.1f, 0.1f, 0.1f);
        p.Rotate(0.0f, m_scale, 0.0f);
        p.WorldPos(-0.8f, -1.0f, 12.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);

        m_DSGeomPassTech.SetWVP(p.GetWVPTrans()); // 设置MVP和M矩阵到shader
        m_DSGeomPassTech.SetWorldMatrix(p.GetWorldTrans());

        m_mesh.Render(); //渲染网格(render the mesh):内部会绑定模型的纹理单元uniform和顶点属性VAO等(但用的shader不是内部提供的)
    }

    void DSLightPass()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 绑定当前读取的fbo为 m_gbuffer
        m_gbuffer.BindForReading(); // 这里只是把g-buffer拷贝到default framebuffer

        GLint HalfWidth = (GLint)(WINDOW_WIDTH / 2.0f);
        GLint HalfHeight = (GLint)(WINDOW_HEIGHT / 2.0f);

		// 设置源fbo的各个附件为当前读取附件 glReadBuffer
        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
		// 目标位置和大小调整  1280x1024  --> 690x512  颜色附件 并且linear插值 !!
		// 第 9 个参数表示我们是否要从颜色、深度或模板缓冲区中读取， 并且可以取值 GL_COLOR_BUFFER_BIT、GL_DEPTH_BUFFER_BIT 或 GL_STENCIL_BUFFER_BIT
		// 最后一个参数确定 OpenGL 处理可能缩放的方式（当源参数和目标参数的维度不同时），可以是 GL_NEAREST 或 GL_LINEAR（看起来比 GL_NEAREST 更好，但需要更多的计算资源）。
		//
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);


        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, HalfHeight, HalfWidth, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);


        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, HalfWidth, HalfHeight, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, HalfWidth, 0, WINDOW_WIDTH, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }


        void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE State)
        {
                switch (OgldevKey) {
                case OGLDEV_KEY_ESCAPE:
                case OGLDEV_KEY_q:
                        GLUTBackendLeaveMainLoop();
                        break;
                default:
                        m_pGameCamera->OnKeyboard(OgldevKey);
                }
        }


        virtual void PassiveMouseCB(int x, int y)
        {
                m_pGameCamera->OnMouse(x, y);
        }


private:

	DSGeomPassTech m_DSGeomPassTech;
    Camera* m_pGameCamera;
    float m_scale;
    BasicMesh m_mesh;
    PersProjInfo m_persProjInfo;
    GBuffer m_gbuffer;
};


int main(int argc, char** argv)
{
//    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 35")) {
        return 1;
    }

    SRANDOM;

    Tutorial35* pApp = new Tutorial35();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;

    return 0;
}
