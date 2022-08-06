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

    Tutorial 37 - Deferred Shading - Part 3
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_glut_backend.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_lights_common.h"
#include "gbuffer.h"
#include "null_technique.h"
#include "ds_geom_pass_tech.h"
#include "ds_point_light_pass_tech.h"
#include "ds_dir_light_pass_tech.h"
#include "filesystem.h"


#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1024

class Tutorial37 : public ICallbacks, public OgldevApp
{
public:

    Tutorial37()
    {
        m_pGameCamera = NULL;
        m_scale = 0.0f;

        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;

        InitLights();
        InitBoxPositions();
    }


    ~Tutorial37()
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

        if (!m_DSPointLightPassTech.Init()) {
                printf("Error initializing DSPointLightPassTech\n");
                return false;
        }

        m_DSPointLightPassTech.Enable();

        m_DSPointLightPassTech.SetPositionTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
        m_DSPointLightPassTech.SetColorTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
        m_DSPointLightPassTech.SetNormalTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        m_DSPointLightPassTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!m_DSDirLightPassTech.Init()) {
                printf("Error initializing DSDirLightPassTech\n");
                return false;
        }

        m_DSDirLightPassTech.Enable();

        m_DSDirLightPassTech.SetPositionTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
        m_DSDirLightPassTech.SetColorTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
        m_DSDirLightPassTech.SetNormalTextureUnit(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        m_DSDirLightPassTech.SetDirectionalLight(m_dirLight);
        m_DSDirLightPassTech.SetScreenSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        Matrix4f WVP;
        WVP.InitIdentity();
        m_DSDirLightPassTech.SetWVP(WVP);

        if (!m_nullTech.Init()) 
		{
                return false;
        }

        if (!m_quad.LoadMesh(FileSystem::getPath("Content/quad.obj"))) 
		{
            return false;
        }

        if (!m_box.LoadMesh(FileSystem::getPath("Content/box.obj")))
		{
			return false;
		}

        if (!m_bsphere.LoadMesh(FileSystem::getPath("Content/sphere.obj"))) 
		{
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
        GLUTBackendRun(this);
    }


    virtual void RenderSceneCB()
    {
        CalcFPS();

        m_scale += 0.05f;

        m_pGameCamera->OnRender();

        m_gbuffer.StartFrame();

        DSGeometryPass();

		// 点光源的光球light volume阶段 需要打开模板测试
        // We need stencil to be enabled in the stencil pass to get the stencil buffer
        // updated and we also need it in the light pass because we render the light
        // only if the stencil passes.
        glEnable(GL_STENCIL_TEST);

        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLight); i++)
		{
			// 对于每一个光，我们都做一个模板阶段（标记相关像素），
			// 然后是一个取决于模板值的点光源着色阶段。
			// 需要分别处理每个光源的原因是，
			// 一旦模板值由于其中一个灯而变得大于零，
			// 我们就无法判断另一个也与同一像素重叠的光源是否相关。

			// 清除并更新模板 正面-1 背面+1 (深度测试/深度写入关闭 打开模板测试/总是通过 先模板后深度 )
            DSStencilPass(i);
			// 点光源着色 (模板不为0的元素 关闭深度测试,开启模板测试,开启混合1+1)
            DSPointLightPass(i);
        }

        // The directional light does not need a stencil test because its volume
        // is unlimited and the final pass simply copies the texture.
        glDisable(GL_STENCIL_TEST);

        DSDirectionalLightPass();

        DSFinalPass();

        RenderFPS();

        glutSwapBuffers();
    }


    void DSGeometryPass()
    {
        m_DSGeomPassTech.Enable();

        m_gbuffer.BindForGeomPass();

        // Only the geometry pass updates the depth buffer
        glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_BLEND); 这里实际是disable了blend的

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        p.Rotate(0.0f, m_scale, 0.0f);

        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boxPositions) ; i++) {
            p.WorldPos(m_boxPositions[i]);
            m_DSGeomPassTech.SetWVP(p.GetWVPTrans());
                m_DSGeomPassTech.SetWorldMatrix(p.GetWorldTrans());
            m_box.Render();
        }

        // When we get here the depth buffer is already populated and the stencil pass
        // depends on it, but it does not write to it.
        glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
    }

    void DSStencilPass(unsigned int PointLightIndex)
    {
		/* 
		管线流程-- 先模板测试 后深度测试 
		  
		比较函数：
			总是通过
			总是失败
			小于/大于
			小于/大于或等于
			平等的
			不相等

		根据模板测试和深度测试的结果，定义一个动作(对存储的模板值的模板操作)
			保持模板值不变
			将模板值替换为零
			增加/减少模板值
			反转模板值的位

		每种情况配置不同的操作：
			模板测试失败
			深度测试失败(模板测试通过)
			深度测试成功

		正面和反面:
			为每个多边形的两个面配置不同的模板测试和模板操作。
			例如，将正面的比较函数设置为“小于”，参考值为 3，
			         而背面的比较函数为“相等”，参考值为 5

		模板缓存和深度缓冲
			它不是一个单独的缓冲区，而是深度缓冲区的一部分。
			可以在每个像素中拥有 24 或 32 位深度和 8 位模板的深度/模板缓冲区。


		因为我们只对写入模板缓冲区感兴趣，所以我们使用空片段着色器

		*/

        m_nullTech.Enable();

        // Disable color/depth write and enable stencil
        m_gbuffer.BindForStencilPass();

        glEnable(GL_DEPTH_TEST);// 使能深度测试 但是关闭了深度写入 
		glDepthMask(GL_FALSE);

		glDisable(GL_CULL_FACE); // 注意!! 关闭背面剔除!! 必须使用双面

		glClear(GL_STENCIL_BUFFER_BIT); // 注意!!(每个点光源) 只清除深度模板缓冲区上的模板部分

        // We need the stencil test to be enabled but we want it
        // to succeed always. Only the depth test matters.

        glStencilFunc(GL_ALWAYS, 0, 0);
		
		//
		// 对方方法---参考值(clamp到,0~2^n-1,n是模板缓存位数)---ref和储存值对比前的mask
		// void glStencilFunc(	GLenum func, GLint ref, GLuint mask);

		//
		// 哪一个面--模板失败--深度失败(模板通过)--深度通过
		// glStencilOpSeparate(	GLenum face,GLenum sfail, GLenum dpfail, GLenum dppass);

		// 根据深度测试是否失败, 失败的话 背面+1 正面-1, 其他情况保持不变 (结果模板只有正数的地方 才有光源)
        glStencilOpSeparate(GL_BACK,   GL_KEEP, GL_INCR_WRAP,  GL_KEEP);// +1 背面
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP,  GL_KEEP);// -1 正面

        Pipeline p;
        p.WorldPos(m_pointLight[PointLightIndex].Position);
		float BBoxScale = CalcPointLightBSphere(m_pointLight[PointLightIndex]);
		p.Scale(BBoxScale, BBoxScale, BBoxScale);
		p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
		p.SetPerspectiveProj(m_persProjInfo);

        m_nullTech.SetWVP(p.GetWVPTrans());
        m_bsphere.Render();
    }


    void DSPointLightPass(unsigned int PointLightIndex)
    {
		// 绑定fbo中的颜色附件4作为输出(颜色附件4只在开始时候clear一次/StartFrame)
		// g-buffer作为输入纹理
		m_gbuffer.BindForLightPass();

        m_DSPointLightPassTech.Enable();
        m_DSPointLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());

		// 将模板测试配置为仅在像素的模板值不为零时通过。

		// 光体之外的所有对象像素都将无法通过模板测试，
		// 我们将在一个非常小的像素子集上计算光照
		// 这些像素实际上被光球覆盖。
		//
        glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // mask是0xFF 

        glDisable(GL_DEPTH_TEST); // 不用深度测试
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD); // blend 1+1 
        glBlendFunc(GL_ONE, GL_ONE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);    // 注意!! 剔除正面, 避免两次渲染 和摄像机进入光球没有效果

		// 启用正面多边形的剔除。因为相机可能在光球(light volume)内，
		// 如果我们像往常一样进行背面剔除，我们将在退出其体积之前看不到光。

        Pipeline p;
        p.WorldPos(m_pointLight[PointLightIndex].Position);
        float BBoxScale = CalcPointLightBSphere(m_pointLight[PointLightIndex]);
		p.Scale(BBoxScale, BBoxScale, BBoxScale);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        m_DSPointLightPassTech.SetWVP(p.GetWVPTrans());
        m_DSPointLightPassTech.SetPointLight(m_pointLight[PointLightIndex]);
        m_bsphere.Render();

        glCullFace(GL_BACK);
		glDisable(GL_BLEND);
    }


    void DSDirectionalLightPass()
    {
		m_gbuffer.BindForLightPass();

		m_DSDirLightPassTech.Enable();
		m_DSDirLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

		m_quad.Render();

		glDisable(GL_BLEND);
    }


    void DSFinalPass()
    {
		/*
			为什么在 G-Buffer 中添加中间颜色缓冲区而不是直接渲染到屏幕:

			几何阶段渲染到G-Buffer, 需要深度缓冲, 但是不能直接用默认缓冲的深度(没有接口获取深度附件的句柄)

		*/
		m_gbuffer.BindForFinalPass();
		glBlitFramebuffer(
			0, 0, 
			WINDOW_WIDTH, WINDOW_HEIGHT,
			0, 0, 
			WINDOW_WIDTH, WINDOW_HEIGHT, 
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);
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

    // The calculation solves a quadratic equation (see http://en.wikipedia.org/wiki/Quadratic_equation)
    float CalcPointLightBSphere(const PointLight& Light)
    {
        float MaxChannel = MAX(MAX(Light.Color.x, Light.Color.y), Light.Color.z);

        float ret = (-Light.Attenuation.Linear + sqrtf(Light.Attenuation.Linear * Light.Attenuation.Linear - 4 * Light.Attenuation.Exp * (Light.Attenuation.Exp - 256 * MaxChannel * Light.DiffuseIntensity)))
                    /
                    (2 * Light.Attenuation.Exp);

        return ret;
    }

    void InitLights()
    {
        m_spotLight.AmbientIntensity = 0.0f;
        m_spotLight.DiffuseIntensity = 0.9f;
        m_spotLight.Color = COLOR_WHITE;
        m_spotLight.Attenuation.Linear = 0.01f;
        m_spotLight.Position  = Vector3f(-20.0, 20.0, 5.0f);
        m_spotLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        m_spotLight.Cutoff =  20.0f;

        m_dirLight.AmbientIntensity = 0.1f;
        m_dirLight.Color = COLOR_CYAN;
        m_dirLight.DiffuseIntensity = 0.5f;
        m_dirLight.Direction = Vector3f(1.0f, 0.0f, 0.0f);

        m_pointLight[0].DiffuseIntensity = 0.2f;
        m_pointLight[0].Color = COLOR_GREEN;
        m_pointLight[0].Position = Vector3f(0.0f, 1.5f, 5.0f);
        m_pointLight[0].Attenuation.Constant = 0.0f;
        m_pointLight[0].Attenuation.Linear = 0.0f;
        m_pointLight[0].Attenuation.Exp = 0.3f;

        m_pointLight[1].DiffuseIntensity = 0.2f;
        m_pointLight[1].Color = COLOR_RED;
        m_pointLight[1].Position = Vector3f(2.0f, 0.0f, 5.0f);
        m_pointLight[1].Attenuation.Constant = 0.0f;
        m_pointLight[1].Attenuation.Linear = 0.0f;
        m_pointLight[1].Attenuation.Exp = 0.3f;

        m_pointLight[2].DiffuseIntensity = 0.2f;
        m_pointLight[2].Color = COLOR_BLUE;
        m_pointLight[2].Position = Vector3f(0.0f, 0.0f, 3.0f);
        m_pointLight[2].Attenuation.Constant = 0.0f;
        m_pointLight[2].Attenuation.Linear = 0.0f;
        m_pointLight[2].Attenuation.Exp = 0.3f;
    }


    void InitBoxPositions()
    {
        m_boxPositions[0] = Vector3f(0.0f, 0.0f, 5.0f);
        m_boxPositions[1] = Vector3f(6.0f, 1.0f, 10.0f);
        m_boxPositions[2] = Vector3f(-5.0f, -1.0f, 12.0f);
        m_boxPositions[3] = Vector3f(4.0f, 4.0f, 15.0f);
        m_boxPositions[4] = Vector3f(-4.0f, 2.0f, 20.0f);
    }

    DSGeomPassTech m_DSGeomPassTech;
    DSPointLightPassTech m_DSPointLightPassTech;
    DSDirLightPassTech m_DSDirLightPassTech;
    NullTechnique m_nullTech;
    Camera* m_pGameCamera;
    float m_scale;
    SpotLight m_spotLight;
    DirectionalLight m_dirLight;
    PointLight m_pointLight[3];
    BasicMesh m_box;
    BasicMesh m_bsphere;
    BasicMesh m_quad;
    PersProjInfo m_persProjInfo;
    GBuffer m_gbuffer;
    Vector3f m_boxPositions[5];
};


int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv, true, false);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 37")) {
        return 1;
    }

    Tutorial37* pApp = new Tutorial37();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;

    return 0;
}
