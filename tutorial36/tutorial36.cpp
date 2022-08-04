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

    Tutorial 36 - Deferred Shading - Part 2
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ds_geom_pass_tech.h"
#include "ds_point_light_pass_tech.h"
#include "ds_dir_light_pass_tech.h"
#include "ogldev_glut_backend.h"
#include "ogldev_basic_mesh.h"
#include "gbuffer.h"
#include "ogldev_lights_common.h"
#include "FileSystem.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1024

class Tutorial36 : public ICallbacks, public OgldevApp
{
public:

    Tutorial36()
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


    ~Tutorial36()
    {
        SAFE_DELETE(m_pGameCamera);
    }

    bool Init()
    {
        if (!m_gbuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }

 

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT,
			Vector3f(-0.46f, -1.028f, -30.97f), // 不使用默认的初始位置， 会导致相机在光球内
			Vector3f(0.0f, 0.0f, 1.0f),
			Vector3f(0.0f, 1.0f, 0.0f));


        if (!m_DSGeomPassTech.Init()) 
		{
            printf("Error initializing DSGeomPassTech\n");
            return false;
        }

		m_DSGeomPassTech.Enable();
		m_DSGeomPassTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

        if (!m_DSPointLightPassTech.Init()) 
		{
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
		// !! 注意 这里面默认启动了cull face背面剔除
		//    几何阶段 和 光照阶段(画球体 或者 画全屏quad) 画球体会被cull了一半

		/*
		
		第一个问题是，
			当相机进入光量时，光会消失。 
			原因是我们只渲染边界球体的正面(背面剔除，也避免了两次渲染光照)，
			因此一旦进入球体就会被剔除。 
			如果我们禁用背面剔除，
			那么由于混合，我们将在球体外部（因为我们将渲染两个面）时获得增加的光线，
			而在内部（仅渲染背面时）只有一半。

		第二个问题是边界球体并没有真正限制光，
			有时在它之外的物体也会被照亮，(在三维空间中, 光体积/光球体以外的地方)
			因为球体在屏幕空间中覆盖了它们，(因为球体投影到屏幕空间，覆盖了他们)
			所以我们计算它们的光照。
		*/
    }


    virtual void RenderSceneCB()
    {
        CalcFPS();

        m_scale += 0.05f;

        m_pGameCamera->OnRender();

        DSGeometryPass();

		// 使能Blend 1+1 绑定G-Buffer所有纹理到纹理单元
        BeginLightPasses();

		//  点光源 ---- 渲染一个粗糙的球体模型，其中心位于光源处。球体的大小将根据光的强度来设置
		//           ---- 减少了必须着色的像素数量。我们不计算小光源对场景中所有对象的影响，而是仅考虑其局部附近
        DSPointLightsPass(); 

		// 定向光 ----  所有屏幕像素都会受到它的影响。在这种情况下，简单地绘制一个全屏四边形
        DSDirectionalLightPass();

        RenderFPS();

        glutSwapBuffers();
    }


    void DSGeometryPass()
    {
        m_DSGeomPassTech.Enable();

        m_gbuffer.BindForWriting();

        // 深度测试限制为几何阶段  
        glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		// !! 清除深度缓冲区之前启用写入深度缓冲区。
		//    如果深度掩码设置为 FALSE，glClear() 不会触及深度缓冲区。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// !! 几何阶段 不能打开混合
        glDisable(GL_BLEND);

        Pipeline p;
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        p.Rotate(0.0f, m_scale, 0.0f);

        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_boxPositions) ; i++) 
		{
            p.WorldPos(m_boxPositions[i]);
            m_DSGeomPassTech.SetWVP(p.GetWVPTrans());
                m_DSGeomPassTech.SetWorldMatrix(p.GetWorldTrans());
            m_box.Render();
        }

		// 到达这里时，深度缓冲区已经被填充
		
		// 模板阶段 深度缓冲，但它不会写入它。
		glDepthMask(GL_FALSE);		// 关闭深度写入
        glDisable(GL_DEPTH_TEST);	// 关闭深度测试 
    }


    void BeginLightPasses()
    {
		// 每个光照都叠加起来  点光源画的球体 和 方向光画的quad 光照计算叠加起来
		// --- 每个光源都由其自己的绘制调用处理, 每个 FS 调用只处理单个光源
        glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);  // !!   1 +  1 

		// 绑定G-Buffer的所有纹理 
        m_gbuffer.BindForReading(); 
        glClear(GL_COLOR_BUFFER_BIT);
    }


    void DSPointLightsPass()
    {
        m_DSPointLightPassTech.Enable();
        m_DSPointLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());

        Pipeline p; // 相机的参数不变
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
		//glDisable(GL_CULL_FACE);

		//for (unsigned int i = 0; i < 1; i++) // 调试只看一个点光源, 比较清晰
		for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLight); i++) 
		{
            m_DSPointLightPassTech.SetPointLight(m_pointLight[i]);

			// 根据光的参数计算球体的大小。
			float BSphereScale = CalcPointLightBSphere(m_pointLight[i]);

			printf("BSphereScale =%f  Position(%f,%f,%f) zFar %f\n", 
				BSphereScale,
				m_pointLight[i].Position.r, m_pointLight[i].Position.g, m_pointLight[i].Position.b, 
				m_persProjInfo.zFar );

			// 要绘制的球体的位置和大小 
            p.WorldPos(m_pointLight[i].Position);
			p.Scale(BSphereScale, BSphereScale, BSphereScale);

            m_DSPointLightPassTech.SetWVP(p.GetWVPTrans());
            m_bsphere.Render();
        }

    }

	// 处理定向光（我们只支持一种这样的光源)
    void DSDirectionalLightPass()
    {
		m_DSDirLightPassTech.Enable();
		m_DSDirLightPassTech.SetEyeWorldPos(m_pGameCamera->GetPos());

		Matrix4f WVP;
		WVP.InitIdentity();
		m_DSDirLightPassTech.SetWVP(WVP);
		m_quad.Render(); // 只是绘制一个矩形
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
		/*
			Light = {R G B}  值是0到1  
			Intensity  值是0到1 
			C = max(R,G,B)
			Attenuation = Constant + Linear * Distance + Exp * Distance^2 
			C * Intensity * 1 / Attenuation  = threshold 
			threshold = 10.0/256.0
		*/
        float MaxChannel = MAX(MAX(Light.Color.x, Light.Color.y), Light.Color.z);
		//float threshold = 1.0;
		float threshold = 10.0;
        float ret = (-Light.Attenuation.Linear + sqrtf(Light.Attenuation.Linear * Light.Attenuation.Linear - 4 * Light.Attenuation.Exp * (Light.Attenuation.Exp - (256 / threshold)* MaxChannel * Light.DiffuseIntensity)))
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
        m_dirLight.Color = COLOR_CYAN; // 青色 0,1,1
        m_dirLight.DiffuseIntensity = 0.5f;
        m_dirLight.Direction = Vector3f(1.0f, 0.0f, 0.0f);

#define PL_DiffuseIntensity 0.8f

        m_pointLight[0].DiffuseIntensity = PL_DiffuseIntensity;
        m_pointLight[0].Color = COLOR_GREEN; // 绿色 三个点光源是 R G B单色
        m_pointLight[0].Position = Vector3f(0.0f, 1.5f, 3.0f); // Vector3f(0.0f, 1.5f, 5.0f);  //Vector3f(0.0f, 8.0f, 5.0f);  
		/* 
		   参数   ---DiffuseIntensity =0.8, Constant = 0, Linear = 0, Exp = 0.3f 
		   计算出---光球半径是8.2f
		   
		   由于, 在边缘处光颜色值为 threshold

		   若有一个盒子在 0.0f, 0.0f, 5.0f,  光源Position在 0.0f, 8.0f, 5.0f的话,  这个盒子几乎就在边缘了
		   光的颜色就是 threshold = 10.0/256.0 
		   再乘以纹理采样颜色(0~1.0) 就更加小了 几乎看不出来

			参数   ---DiffuseIntensity =0.2(减少了), Constant = 0, Linear = 0, Exp = 0.3f
			计算出---光球半径是4.0f
		 */
        m_pointLight[0].Attenuation.Constant = 0.0f; // 衰减系数都一样 
        m_pointLight[0].Attenuation.Linear = 0.0f;  // 只是位置和颜色不同
        m_pointLight[0].Attenuation.Exp = 0.3f; // 全部为0会导致 CalcPointLightBSphere 光球大小为nan
																	
        m_pointLight[1].DiffuseIntensity = PL_DiffuseIntensity;
        m_pointLight[1].Color = COLOR_RED;
        m_pointLight[1].Position = Vector3f(-4.0f, 0.0f, 10.0f);
        m_pointLight[1].Attenuation.Constant = 0.0f;
        m_pointLight[1].Attenuation.Linear = 0.0f;
        m_pointLight[1].Attenuation.Exp = 0.3f; // 如果太小导致球体太大 背面都超出了视椎体

        m_pointLight[2].DiffuseIntensity = PL_DiffuseIntensity;
        m_pointLight[2].Color = COLOR_BLUE;
        m_pointLight[2].Position = Vector3f(4.0f, 5.0f, 13.0f);
        m_pointLight[2].Attenuation.Constant = 0.0f;
        m_pointLight[2].Attenuation.Linear = 0.0f;
        m_pointLight[2].Attenuation.Exp = 0.3f;
    }


    void InitBoxPositions()
    {
        m_boxPositions[0] = Vector3f(0.0f, 0.0f, 5.0f); // 靠近绿色点光源 (0.0f, 1.5f, 3.0f)
        m_boxPositions[1] = Vector3f(6.0f, 1.0f, 10.0f);
        m_boxPositions[2] = Vector3f(-5.0f, -1.0f, 12.0f);//  靠近红色点光源 -4.0f, 0.0f, 10.0f
        m_boxPositions[3] = Vector3f(4.0f, 4.0f, 15.0f);// 靠近蓝色点光源 (4.0f, 5.0f, 13.0f)
        m_boxPositions[4] = Vector3f(-4.0f, 2.0f, 20.0f);
    }

    DSGeomPassTech m_DSGeomPassTech;
    DSPointLightPassTech m_DSPointLightPassTech;
    DSDirLightPassTech m_DSDirLightPassTech;
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

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 36")) {
        return 1;
    }

    Tutorial36* pApp = new Tutorial36();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;

    return 0;
}
