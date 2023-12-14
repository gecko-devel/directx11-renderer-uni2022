//--------------------------------------------------------------------------------------
// File: DX11 Framework.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Shader vars
//--------------------------------------------------------------------------------------
Texture2D texDiffuse : register(t0);
Texture2D texNorm : register(t1);
Texture2D texSpec : register(t2);

SamplerState sampLinear : register(s0);

struct GlobalLight
{
    float4 AmbLight;
    float4 DiffLight;
    float4 SpecLight;
    float4 DirectionToLight;
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
    
    bool hasAlbedoTexture;
    bool hasNormalMapTexture;
    bool hasSpecularMapTexture;
    
    float specularPower;

    float3 EyePosW;
    int numPointLights;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
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
    //float4 litColor;
    float4 ambient = float4(0.0, 0.0, 0.0, 0.0);
    float4 diffuse = float4(0.0, 0.0, 0.0, 0.0);
    float4 specular = float4(0.0, 0.0, 0.0, 0.0);
    
    // Ambient Lighting
    ambient = mGlobalLight.AmbLight * AmbMat;
    
    // ----------------
    // GlobalLight
    // ----------------
    // Diffuse Lighting
    float4 potentialDiff = mGlobalLight.DiffLight * DiffMat;
    float difPercent = max(dot(normalize(mGlobalLight.DirectionToLight), normalize(input.NormalW)), 0);
    float DiffuseAmount = difPercent * potentialDiff;
    diffuse += DiffuseAmount * (DiffMat * mGlobalLight.DiffLight);
    
    // Specular lighting
    float4 potentialSpecular;
    
    if (hasSpecularMapTexture)
        potentialSpecular = mGlobalLight.SpecLight * texSpec.Sample(sampLinear, input.TexCoord);
    else
        potentialSpecular = mGlobalLight.SpecLight;
     
    float3 viewerDir = normalize(EyePosW - input.PosW);
    float3 reflectDir = reflect(-mGlobalLight.DirectionToLight, normalize(input.NormalW));
    reflectDir = normalize(reflectDir);
    float specularIntensity = pow(max(dot(reflectDir, viewerDir), 0), specularPower);
    specular += (potentialSpecular * specularIntensity);
    
    // ----------------
    // PointLight
    // ----------------
    
    for (int i = 0; i < -1; i++)
    {
        float3 directionToPointLight = normalize(PointLights[i].pos - input.PosW);
        float distanceToPointLight = length(PointLights[i].pos - input.PosW);
        float pointLightIntensity = 1 / 1 + pow(PointLights[i].attenuation * distanceToPointLight, 2);
    
        // Diffuse
        difPercent = max(dot(normalize(directionToPointLight), normalize(input.NormalW)), 0);
        DiffuseAmount = difPercent * potentialDiff;
        diffuse += (DiffuseAmount * (DiffMat * PointLights[i].color)) * pointLightIntensity;
    
        // Specular
        potentialSpecular = PointLights[i].color * texSpec.Sample(sampLinear, input.TexCoord);
        reflectDir = normalize(reflect(-directionToPointLight, normalize(input.NormalW)));
        specularIntensity = pow(max(dot(reflectDir, viewerDir), 0), specularPower);
        specular += (potentialSpecular * specularIntensity) * pointLightIntensity;
    }
    
    // ----------------
    // Texturing
    // ----------------
    float4 textureColor = float4(0.0, 0.0, 0.0, 0.0);
    
    if (hasAlbedoTexture == true)
    {
        textureColor = texDiffuse.Sample(sampLinear, input.TexCoord);
    }
    
    return textureColor * (ambient + diffuse) + specular;
}
