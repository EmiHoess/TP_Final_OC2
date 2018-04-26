//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4   g_vDiffuse;                // Material diffuse color
float4x4 g_mViewProj;
float4x4 g_mWorld;					// World matrix for object
float4x4 g_mWorldViewProjection;	// World * View * Projection matrix
texture  g_txScene;


sampler g_samScene =
sampler_state
{
    Texture = <g_txScene>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = None;
};


void VertScene( float4 Pos : POSITION,
                float3 Normal : NORMAL,
                float2 Tex : TEXCOORD0,
                out float4 oPos : POSITION,
                out float2 oTex : TEXCOORD0,
                out float4 Diffuse : COLOR0 )
{
    oPos = mul( Pos, g_mWorldViewProjection );

    //float3 N = normalize( mul( Normal, (float3x3)g_mWorld ) );
    //Diffuse = saturate( dot( (float3)N, float3( 0.0f, 0.0f, -1.0f ) ) ) * g_vDiffuse;
	float4 diffuse2 = float4(1,0,0,1);
	
	float lighting = max( 0.3, saturate( dot( Normal, float3( 0.0f, -1.0f, 0.0f ) ) ) );
	Diffuse = lighting*diffuse2;
    oTex = Tex;
}

struct VS_OUTPUT
{
    float4  Pos     : POSITION;
    float4  Diffuse : COLOR0;
    float2  Tex0    : TEXCOORD0;
};

VS_OUTPUT VertSceneF( float4 Pos : POSITION,
                     float3 Normal : NORMAL,
                     float2 Tex0 : TEXCOORD0 )
{
    VS_OUTPUT o;
    
    o.Pos = mul( Pos, g_mWorld );
    o.Pos = mul( o.Pos, g_mViewProj );
    o.Tex0 = Tex0;
    float3 N = normalize( mul( Normal, (float3x3)g_mWorld ) );

    // Always fully lit the floor
    o.Diffuse = 1.0f;
    
    return o;
}


float4 PixScene( float2 Tex : TEXCOORD0,
                 float4 Diffuse : COLOR0 ) : COLOR0
{
    return tex2D( g_samScene, Tex ) * Diffuse;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {
        VertexShader = compile vs_2_0 VertScene();
        PixelShader = compile ps_2_0 PixScene();
    }
}

technique RenderFloor
{
    pass P0
    {
        VertexShader = compile vs_2_0 VertSceneF();
        PixelShader = compile ps_2_0 PixScene();
    }
}
