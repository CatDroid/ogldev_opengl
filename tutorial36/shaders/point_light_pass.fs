#version 330

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct DirectionalLight
{
    BaseLight Base;
    vec3 Direction;
};

struct Attenuation
{
    float Constant;
    float Linear;
    float Exp;
};

struct PointLight
{
    BaseLight Base;
    vec3 Position;
    Attenuation Atten;
};

struct SpotLight
{
    PointLight Base;
    vec3 Direction;
    float Cutoff;
};

uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;
uniform sampler2D gNormalMap;
uniform DirectionalLight gDirectionalLight;
uniform PointLight gPointLight;
uniform SpotLight gSpotLight;
uniform vec3 gEyeWorldPos;
uniform float gMatSpecularIntensity;
uniform float gSpecularPower;
uniform int gLightType;
uniform vec2 gScreenSize;

vec4 CalcLightInternal(BaseLight Light,
					   vec3 LightDirection,
					   vec3 WorldPos,
					   vec3 Normal)
{
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0);
    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0.0) {
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0);

        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal)); // Phong模型 计算反射向量
        float SpecularFactor = dot(VertexToEye, LightReflect);  // 光反射向量和视线的夹角      
        if (SpecularFactor > 0.0) {
            SpecularFactor = pow(SpecularFactor, gSpecularPower);
            SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0);
        }
    }

    return (AmbientColor + DiffuseColor + SpecularColor);
}
#if 0 
bool isnan( float val )
{
  return ( val < 0.0 || 0.0 < val || val == 0.0 ) ? false : true;
  // important: some nVidias failed to cope with version below.
  // Probably wrong optimization.
  /*return ( val <= 0.0 || 0.0 <= val ) ? false : true;*/
}
#endif 

vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base,
							 gDirectionalLight.Direction,
							 WorldPos,
							 Normal);
}

vec4 CalcPointLight(vec3 WorldPos, vec3 Normal)
{
    vec3 LightDirection = WorldPos - gPointLight.Position; // 从光源指向表面点
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 Color = CalcLightInternal(gPointLight.Base, LightDirection, WorldPos, Normal);

    float AttenuationFactor =  gPointLight.Atten.Constant +
                         gPointLight.Atten.Linear * Distance +
                         gPointLight.Atten.Exp * Distance * Distance;

    AttenuationFactor = max(1.0, AttenuationFactor);

    return Color / AttenuationFactor;
}

float CalcAttenuation(vec3 WorldPos)
{
    vec3 LightDirection = WorldPos - gPointLight.Position; 
    float Distance = length(LightDirection);

	float AttenuationFactor =  gPointLight.Atten.Constant +
                         gPointLight.Atten.Linear * Distance +
                         gPointLight.Atten.Exp * Distance * Distance;

    AttenuationFactor = max(1.0, AttenuationFactor);

	return  1.0 / AttenuationFactor; // 1.0 ---> 接近0.0
}


vec2 CalcTexCoord()
{
	// 计算G-Buffer的纹理坐标
	// 其 XY 分量中的屏幕空间坐标(像素单位)
	// 其 Z 分量中的像素深度(0~1)
	// 其 W 分量中的 1/W
	// 
    return gl_FragCoord.xy / gScreenSize;
}


out vec4 FragColor;

void main()
{
    vec2 TexCoord = CalcTexCoord();
	vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
	vec3 Color = texture(gColorMap, TexCoord).xyz;
	vec3 Normal = texture(gNormalMap, TexCoord).xyz;
	Normal = normalize(Normal);

    // FragColor = vec4(Color, 1.0) +  CalcPointLight(WorldPos, Normal)*0.0000001; 
	// 这里有三个点光源 会叠加起来 所以会变成白色
	FragColor = vec4(Color, 1.0) * CalcPointLight(WorldPos, Normal);

	// 在物体以外的地方 WorldPos都是0 只是每个灯源的LightPos不一样, 所以atten不一样,但都是均匀的
	// 在有物体的地方 WorldPos就不是0了

	//float atten = CalcAttenuation(WorldPos);
	//FragColor = FragColor*0.0000001 +vec4(gPointLight.Base.Color* atten, 1.0) ;

	//atten =  isnan(atten) ? 0.0: 1.0 ; // 没有明显的nan 
	//FragColor = FragColor*0.0000001 + vec4(atten, atten, atten, 1.0); 
}
