//--------------------------------------------------------------------------------------
// File: DX11 Framework.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

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
    float3 DirToLight;
    
    float T;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float3 PosW : POSITION0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float3 Pos : POSITION, float3 Normal : NORMAL, float3 PosW : POSITION0 )
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
    float4 NormalW = mul(Normal, World);
    NormalW = normalize(NormalW);
    
    float4 potentialDiff = DiffLight * DiffMat;
    float difPercent = max(dot(normalize(DirToLight), normalize(NormalW)), 0);
    
    float DiffuseAmount = difPercent * potentialDiff;
    
    output.Color = DiffuseAmount * (DiffMat * DiffLight);
    
    // Ambient Lighting
    output.Color += AmbLight * AmbMat;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    return input.Color; //* input.PosW.y * sin(T);
}
