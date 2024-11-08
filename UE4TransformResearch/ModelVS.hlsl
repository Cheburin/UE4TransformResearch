cbuffer cbMain : register( b0 )
{
	matrix    g_mWorld;                         // World matrix
	matrix    g_mView;                          // View matrix
	matrix    g_mProjection;                    // Projection matrix
	matrix    g_mWorldViewProjection;           // WVP matrix
	matrix    g_mWorldView;                     // WV matrix
	matrix    g_mInvView;                       // Inverse of view matrix

	matrix    g_mObject1;                // VP matrix
	matrix    g_mObject1WorldView;                       // Inverse of view matrix
	matrix    g_mObject1WorldViewProjection;                       // Inverse of view matrix

	matrix    g_mObject2;                // VP matrix
	matrix    g_mObject2WorldView;                       // Inverse of view matrix
	matrix    g_mObject2WorldViewProjection;                       // Inverse of view matrix

	float4    g_vFrustumNearFar;              // Screen resolution
	float4    g_vFrustumParams;              // Screen resolution
	float4    g_viewLightPos;                   //
};

struct PosNormalTex2d
{
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD0;
    float3 pos : SV_Position;
};

struct ClipPosNormalTex2d
{
    float3 normal         : TEXCOORD1;   // Normal vector in world space
    float2 tex            : TEXCOORD2;
    float4 clip_pos       : SV_POSITION; // Output position
};

struct ExpandPos
{
    float4 clip_pos       : SV_POSITION; // Output position
};

///////////////////////////////////////////////////////////////////////////////////////////////////

ClipPosNormalTex2d MODEL_VERTEX( in PosNormalTex2d i )
{
    ClipPosNormalTex2d output;

    output.normal = normalize( i.normal );

    output.tex = i.tex;

    output.clip_pos = mul( float4( i.pos, 1.0 ), g_mWorldViewProjection );
    
    return output;
}; 

///////////////////////////////////////////////////////////////////////////////////////////////////
float4 QUAD_VS(uint VertexID : SV_VERTEXID):SV_POSITION
{
  return float4( 0, 0, 0, 1.0 );
} 

ClipPosNormalTex2d _p(float x, float y){
  ClipPosNormalTex2d p;
  p.tex = float2(x*0.5+0.5, 0.5-y*0.5);
  p.normal = float3(0,0,0);
  p.clip_pos = float4(x, y, 0.5, 1); 
  return p;
}

[maxvertexcount(4)]
void QUAD_GS(point ExpandPos pnt[1], uint primID : SV_PrimitiveID,  inout TriangleStream<ClipPosNormalTex2d> triStream )
{
	triStream.Append(_p(-1, -1));
	triStream.Append(_p(-1,  1));
	triStream.Append(_p( 1, -1));
	triStream.Append(_p( 1,  1));
    triStream.RestartStrip();
}