Texture2D colorMap  : register( t0 );

SamplerState linearSampler : register( s0 );

float4 MODEL_FRAG(
    float3 normal         : TEXCOORD1,
    float2 tex            : TEXCOORD2
):SV_TARGET
{ 
   return float4(1,1,1,1);
};

float4 QUAD_FRAG(
    float3 normal         : TEXCOORD1,
    float2 tex            : TEXCOORD2
):SV_TARGET
{ 
   return colorMap.Sample(linearSampler, tex);
};
