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

SamplerState sampLinear : register(s0);

struct Light
{
    float4 Color;
    // -------------- 16 bytes
    float3 Direction;
    int LightType;
    // -------------- 16 bytes
    // Below is used by point light and spot light
    float3 Position;
    float Attenuation;
    // -------------- 16 bytes
    // Below is ONLY used by spot light
    float SpotAngle;
    // Light type and padding
    float3 _padding;
    // -------------- 16 bytes
};


struct Fog
{
    float4 Color;
    // -------------- 16 bytes
    float Start;
    float Range;
    float2 _padding;
    // -------------- 16 bytes
};

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
    // ----------- 64 bytes
	matrix View;
    // ----------- 64 bytes
	matrix Projection;
    // ----------- 64 bytes
    
    float4 ambientLight;
    // ----------- 16 bytes
    
    Fog fog;
    // ----------- 32 bytes
    
    Light lights[20];
    // ----------- 1280 bytes
    
    float4 AmbMat;
    // ----------- 16 bytes
    float4 DiffMat;
    // ----------- 16 bytes
    float4 SpecMat;
    // ----------- 16 bytes
    
    float3 EyePosW;
    
    bool hasAlbedoTexture;
    // ----------- 16 bytes
    
    bool hasSpecularMapTexture;
    
    int numLights;
    
    float SpecularPower;

    float1 _padding;
    // ----------- 16 bytes
}

// Functions

float4 Diffuse(Light light, float3 directionToLight, float3 surfaceNormal)
{
    float4 potentialDiff = light.Color * DiffMat;
    float diffPercent = max(dot(directionToLight, surfaceNormal), 0);
    return potentialDiff * diffPercent;
}

// Blinn-Phong specular
float4 Specular(Light light, float3 viewDirection, float3 directionToLight, float3 surfaceNormal, float2 texCoord)
{
    float4 potentialSpecular;
    
    if (hasSpecularMapTexture)
        potentialSpecular = light.Color * texSpec.Sample(sampLinear, texCoord);
    else
        potentialSpecular = light.Color * SpecMat;
    
    
    float3 halfwayVector = normalize(directionToLight + viewDirection);
    float normalDotHalfway = max(0, dot(surfaceNormal, halfwayVector));
    return potentialSpecular * pow(normalDotHalfway, SpecularPower);
    
}

float Attenuation(Light light, float distance)
{   
    return 1.0f / (1.0f + pow(light.Attenuation * distance, 2.0f));
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
    float4 litColor;
    
    float4 ambient = float4(1.0, 1.0, 1.0, 1.0);
    float4 diffuse = float4(0.0, 0.0, 0.0, 0.0);
    float4 specular = float4(0.0, 0.0, 0.0, 0.0);
    
    // Ambient Lighting
    ambient = mGlobalLight.AmbLight * AmbMat;
    
    // ----------------
    // GlobalLight
    // ----------------
    // Diffuse
    float4 potentialDiff = mGlobalLight.DiffLight * DiffMat;
    float difPercent = max(dot(normalize(mGlobalLight.DirectionToLight), normalize(input.NormalW)), 0);
    float DiffuseAmount = difPercent * potentialDiff;
    diffuse += DiffuseAmount * (DiffMat * mGlobalLight.DiffLight);
    
    // Specular
    float4 potentialSpecular;
    
    if (hasSpecularMapTexture)
        potentialSpecular = mGlobalLight.SpecLight * texSpec.Sample(sampLinear, input.TexCoord);
    else
        potentialSpecular = mGlobalLight.SpecLight;
     
    float3 viewerDir = normalize(EyePosW - input.PosW);
    
    // ----------------
    // PointLights
    // ----------------
    
    // Lights
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].LightType)
        {
            case 0: // Directional Light
                {
                    float3 directionToLight = -lights[i].Direction;
        
                    diffuse += Diffuse(lights[i], directionToLight, input.NormalW);
                    specular += Specular(lights[i], viewerDir, directionToLight, input.NormalW, input.TexCoord);    
                }
                break;
            
            case 1: // Point Light
                {
                    float3 lightVector = normalize(lights[i].Position - input.PosW);
                    float distanceToLight = length(lightVector);
                    lightVector = lightVector / distanceToLight;
            
                    float attenuation = Attenuation(lights[i], distanceToLight);
                
                    diffuse += Diffuse(lights[i], lightVector, input.NormalW) * attenuation;
                    specular += Specular(lights[i], viewerDir, lightVector, input.NormalW, input.TexCoord) * attenuation;
                }
                break;
            
            case 2: // Spot Light
                {
                    // Get direction of the light to the current pixel's world position
                    float3 lightVector = lights[i].Position - input.PosW;
                    float distanceToLight = length(lightVector);
        
                    // Falloff as light vector gets close to maximum angle to normal
                    // Most code is derived (not directly copied) from here: https://www.3dgep.com/texturing-lighting-directx-11/#Spotlight_Cone
                    float minCos = cos(lights[i].SpotAngle);
                    float maxCos = (minCos + 1.0f) / 2.0f;
                    float cosAngle = dot(lights[i].Direction, -lightVector);
                    float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
                
                    float attenuation = Attenuation(lights[i], distanceToLight);
                
                    diffuse += Diffuse(lights[i], lightVector, input.NormalW) * attenuation * spotIntensity;
                    specular += Specular(lights[i], viewerDir, lightVector, input.NormalW, input.TexCoord) * attenuation * spotIntensity;
                }
                break;
        }
    }
    
    // Modulate with late add. See verse Frank Luna 8.6 of the Bible to remind yourself what this means.
    litColor = ambient + diffuse;
    
    // Texturing
    // Account for having no texture available by multiplying by texture first and only using the texture
    // if it has one.
    if (hasAlbedoTexture)
    {
        float4 textureColor = texDiffuse.Sample(sampLinear, input.TexCoord);
        litColor *= textureColor;
        
        // Translucency
        litColor.a = textureColor.a * DiffMat.a;

    }
    else
    {
        // Translucency
        litColor.a = DiffMat.a;
    }

    litColor.rgb += specular.rgb;
    
    // The fog is coming
    float distanceToCamera = distance(input.PosW, EyePosW);
    float fogLerp = saturate((distanceToCamera - fog.Start) / fog.Range);
    
    litColor = lerp(litColor, fog.Color, fogLerp);
    
    return litColor;
}