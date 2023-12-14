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

struct PointLight
{
    float4 color;
    float3 pos;
    float radius;
    float attenuation;
    float3 paddinglol;
};

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    
    float4 AmbLight;
    float4 AmbMat;
    
    float4 DiffLight;
    float4 DiffMat;
    
    float4 SpecMat;
    float4 SpecLight;
    float3 EyePosW;
    float SpecPower;
    
    float3 DirToLight;
    
    float T;
    
    PointLight PointLight1;
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
    
    // Diffuse Lighting
    float3 NormalW = mul(Normal, World);
    NormalW = normalize(NormalW);
    
    output.NormalW = NormalW;
    
    float4 potentialDiff = DiffLight * DiffMat;
    float difPercent = max(dot(normalize(DirToLight), normalize(NormalW)), 0);
    
    float DiffuseAmount = difPercent * potentialDiff;
    
    output.Color = DiffuseAmount * (DiffMat * DiffLight);
    
    // Ambient Lighting
    output.Color += AmbLight * AmbMat;
    
    output.TexCoord = texcoord;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    // Ambient Lighting
    input.Color += AmbLight * AmbMat;
    
    
    // ----------------
    // Directional Light
    // ----------------
    
    // Diffuse Lighting
    float4 potentialDiff = DiffLight * DiffMat;
    float difPercent = max(dot(normalize(DirToLight), normalize(input.NormalW)), 0);
    
    float DiffuseAmount = difPercent * potentialDiff;
    
    input.Color += DiffuseAmount * (DiffMat * DiffLight);
    
    // Specular lighting
    
    float4 potentialSpecular = SpecLight * texSpec.Sample(sampLinear, input.TexCoord);
    
    float3 viewerDir = normalize(EyePosW - input.PosW);
    
    float3 reflectDir = reflect(-DirToLight, normalize(input.NormalW));
    reflectDir = normalize(reflectDir);
    
    float specularIntensity = pow(max(dot(reflectDir, viewerDir), 0), SpecPower);
    
    input.Color += (potentialSpecular * specularIntensity);
    
    // ----------------
    // PointLight
    // ----------------
    float3 directionToPointLight = normalize(PointLight1.pos - input.PosW);
    float distanceToPointLight = length(PointLight1.pos - input.PosW);
    float pointLightIntensity = 1 / 1 + pow(PointLight1.attenuation * distanceToPointLight, 2);
    
    // Diffuse
    difPercent = max(dot(normalize(directionToPointLight), normalize(input.NormalW)), 0);
    DiffuseAmount = difPercent * potentialDiff;
    input.Color += (DiffuseAmount * (DiffMat * PointLight1.color)) * pointLightIntensity;
    
    // Specular
    potentialSpecular = PointLight1.color * texSpec.Sample(sampLinear, input.TexCoord);
    reflectDir = reflect(-directionToPointLight, normalize(input.NormalW));
    reflectDir = normalize(reflectDir);
    specularIntensity = pow(max(dot(reflectDir, viewerDir), 0), SpecPower);
    input.Color += (potentialSpecular * specularIntensity) * pointLightIntensity;
    
    // ----------------
    // Texturing
    // ----------------
    float4 textureColor = texDiffuse.Sample(sampLinear, input.TexCoord);
    input.Color *= textureColor;
    
    return input.Color;
}
