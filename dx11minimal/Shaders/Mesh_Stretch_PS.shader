#include <lib/constBuf.shader>

Texture2D inputTexture : register(t2);
SamplerState samplerState : register(s2);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 vpos : POSITION0;
    float4 wpos : POSITION1;
    float4 vnorm : NORMAL0;
    float2 uv : TEXCOORD0;
};

float4 PS(VS_OUTPUT input) : SV_Target
{
    //return float4(1, 0, 1, 1);

    float3 lightDir = normalize(float3(1, -1, 0.25));
    float4 ambientColor = float4(0.15, 0.15, 0.15, 1);
    float specularPower = 2;

    lightDir = -lightDir;

    float3 eye = -(view._m02_m12_m22) * view._m32;
    float3 viewDir = normalize(input.wpos.xyz - eye);

    float3 diffuse = saturate(dot(lightDir, input.vnorm));

    float3 reflectDir = normalize(reflect(lightDir, input.vnorm));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = specularPower * spec;

    float3 tex = inputTexture.Sample(samplerState, input.uv);
    float4 color = gConst[0];

    return float4(tex, 1) * color;
    //return saturate(float4(reflectDir, 1));
    return saturate(float4(tex * (ambientColor.xyz + diffuse + specular), 1));
}