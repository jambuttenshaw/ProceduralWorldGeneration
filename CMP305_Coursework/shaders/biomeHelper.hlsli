
#define MAX_BIOMES 32

struct BiomeMappingBuffer
{
    float pxPerTile;
    uint resolution;
    float blending;
    float padding;
};

struct BiomeTan
{
    float3 shoreColour;
    float3 flatColour;
    float3 flatDetailColour;
    float3 slopeColour;
    float3 cliffColour;
    float3 snowColour;
    
    float3 shallowWaterColour;
    float3 deepWaterColour;
    
    float flatThreshold;
    float cliffThreshold;
    float shoreHeight;
    
    float snowHeight;
    float snowSteepness;
    float snowSmoothing;
    
    float detailThreshold;
    float detailScale;
    
    float steepnessSmoothing;
    float heightSmoothing;
};

// returns uv of the entire biome map
float2 GetBiomeMapUV(float2 pos, BiomeMappingBuffer mappingBuffer)
{
    return pos * mappingBuffer.pxPerTile / mappingBuffer.resolution;
}

uint2 GetBiomeMapLocation(float2 pos, BiomeMappingBuffer mappingBuffer)
{
    return (mappingBuffer.resolution - 1) * GetBiomeMapUV(pos, mappingBuffer);
}

// retuns uv within the biome
float2 GetBiomeUV(float2 pos, BiomeMappingBuffer mappingBuffer)
{
    return frac(float(mappingBuffer.resolution - 1) * GetBiomeMapUV(pos, mappingBuffer));
}

// get how much of this biome should be blended with neighbours
float2 GetBiomeBlend(float2 pos, BiomeMappingBuffer mappingBuffer)
{
    float2 biomeUV = GetBiomeUV(pos, mappingBuffer);
    
    float2 biomeBlend = float2(0.0f, 0.0f);
    
    biomeBlend += step(biomeUV, 0.5f) * (smoothstep(0.0f, 1.0f, biomeUV / mappingBuffer.blending + 0.5f) - 1.0f);
    biomeBlend += step(0.5f, biomeUV) * smoothstep(0.0f, 1.0f, (biomeUV * (1 + mappingBuffer.blending) - 1.0f) / (2.0f * mappingBuffer.blending));
    
    return biomeBlend;
}


// interpolate tan
BiomeTan BlendTans(BiomeTan a, BiomeTan b, float t)
{
    BiomeTan blended;
    blended.shoreColour = lerp(a.shoreColour, b.shoreColour, t);
    blended.flatColour  = lerp(a.flatColour,  b.flatColour,  t);
    blended.flatDetailColour = lerp(a.flatDetailColour, b.flatDetailColour, t);
    blended.slopeColour = lerp(a.slopeColour, b.slopeColour, t);
    blended.cliffColour = lerp(a.cliffColour, b.cliffColour, t);
    blended.snowColour = lerp(a.snowColour, b.snowColour, t);
    
    blended.shallowWaterColour = lerp(a.shallowWaterColour, b.shallowWaterColour, t);
    blended.deepWaterColour = lerp(a.deepWaterColour, b.deepWaterColour, t);
    
    blended.flatThreshold = lerp(a.flatThreshold, b.flatThreshold, t);
    blended.cliffThreshold = lerp(a.cliffThreshold, b.cliffThreshold, t);
    blended.shoreHeight = lerp(a.shoreHeight, b.shoreHeight, t);
    
    blended.snowHeight = lerp(a.snowHeight, b.snowHeight, t);
    blended.snowSteepness = lerp(a.snowSteepness, b.snowSteepness, t);
    blended.snowSmoothing = lerp(a.snowSmoothing, b.snowSmoothing, t);
    
    blended.detailThreshold = lerp(a.detailThreshold, b.detailThreshold, t);
    blended.detailScale = lerp(a.detailScale, b.detailScale, t);
    
    blended.steepnessSmoothing = lerp(a.steepnessSmoothing, b.steepnessSmoothing, t);
    blended.heightSmoothing = lerp(a.heightSmoothing, b.heightSmoothing, t);
    
    return blended;
}

BiomeTan BlendTans(Texture2D biomeMap, StructuredBuffer<BiomeTan> biomeTans, uint2 biomeMapUV, float2 weights)
{
    int b1 = asint(biomeMap.Load(uint3(biomeMapUV + uint2(0,               0),               0)).r);
    int b2 = asint(biomeMap.Load(uint3(biomeMapUV + uint2(sign(weights.x), 0),               0)).r);
    int b3 = asint(biomeMap.Load(uint3(biomeMapUV + uint2(0,               sign(weights.y)), 0)).r);
    int b4 = asint(biomeMap.Load(uint3(biomeMapUV + uint2(sign(weights.x), sign(weights.y)), 0)).r);
    
    return BlendTans(
        BlendTans(biomeTans[b1], biomeTans[b3], abs(weights.y)),
        BlendTans(biomeTans[b2], biomeTans[b4], abs(weights.y)),
        abs(weights.x)
    );
}
