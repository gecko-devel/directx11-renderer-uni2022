//--------------------------------------------------------------------------------------
// File: DX11 Framework.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Shader vars
//--------------------------------------------------------------------------------------
Texture2D texDiffuse : register(t0);
Texture2D texSpec : register(t1);
Texture2D texNorm : register(t2);

//Just another option
//Texture2D texArray[2];


SamplerState sampLinear : register(s0);

struct GlobalLight
{
    float4 AmbLight;
    float4 DiffLight;
    float4 SpecLight;
    float3 DirectionToLight;
    float SpecPower;
};

struct PointLight
{
    float4 color;
    float3 pos;
    float attenuation;
};

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    
    GlobalLight mGlobalLight;
    PointLight PointLights[20];
    
    float4 AmbMat;
    float4 DiffMat;    
    float4 SpecMat;
    
    bool hasTexture;

    float3 EyePosW;
    
    float T;
    int numPointLights;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float3 PosW : POSITION0;
    float3 NormalW : NORMAL0;
    float2 TexCoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(float3 Pos : POSITION, float3 Normal : NORMAL, float2 texcoord : TEXCOORD0)
{
    float4 pos4 = float4(Pos, 1.0f);
    
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul( pos4, World);
    output.PosW = output.Pos;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    
    // Normalise normals
    Normal = normalize(Normal);
    
    float3 NormalW = mul(Normal, World);
    NormalW = normalize(NormalW);
    
    output.NormalW = NormalW;
    output.TexCoord = texcoord;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    // Ambient Lighting
    input.Color += mGlobalLight.AmbLight * AmbMat;
    
    // ----------------
    // GlobalLight
    // ----------------
    
    // Diffuse Lighting
    float4 potentialDiff = mGlobalLight.DiffLight * DiffMat;
    float difPercent = max(dot(normalize(mGlobalLight.DirectionToLight), normalize(input.NormalW)), 0);
    
    float DiffuseAmount = difPercent * potentialDiff;
    
    input.Color += DiffuseAmount * (DiffMat * mGlobalLight.DiffLight);
    
    // Specular lighting
    
    float4 potentialSpecular = mGlobalLight.SpecLight * texSpec.Sample(sampLinear, input.TexCoord);
    
    float3 viewerDir = normalize(EyePosW - input.PosW);
    
    float3 reflectDir = reflect(-mGlobalLight.DirectionToLight, normalize(input.NormalW));
    reflectDir = normalize(reflectDir);
    
    float specularIntensity = pow(max(dot(reflectDir, viewerDir), 0), mGlobalLight.SpecPower);
    
    input.Color += (potentialSpecular * specularIntensity);
    
    // ----------------
    // PointLight
    // ----------------
    
    for (int i = 0; i < numPointLights; i++)
    {
        float3 directionToPointLight = normalize(PointLights[i].pos - input.PosW);
        float distanceToPointLight = length(PointLights[i].pos - input.PosW);
        float pointLightIntensity = 1 / 1 + pow(PointLights[i].attenuation * distanceToPointLight, 2);
    
        // Diffuse
        difPercent = max(dot(normalize(directionToPointLight), normalize(input.NormalW)), 0);
        DiffuseAmount = difPercent * potentialDiff;
        input.Color += (DiffuseAmount * (DiffMat * PointLights[i].color)) * pointLightIntensity;
    
        // Specular
        potentialSpecular = PointLights[i].color * texSpec.Sample(sampLinear, input.TexCoord);
        reflectDir = normalize(reflect(-directionToPointLight, normalize(input.NormalW)));
        specularIntensity = pow(max(dot(reflectDir, viewerDir), 0), mGlobalLight.SpecPower);
        input.Color += (potentialSpecular * specularIntensity) * pointLightIntensity;
    }
    
    // ----------------
    // Texturing
    // ----------------
    if (hasTexture == true)
    {
        float4 textureColor = texDiffuse.Sample(sampLinear, input.TexCoord);
        input.Color *= textureColor;
    }
    
    return input.Color;
}
