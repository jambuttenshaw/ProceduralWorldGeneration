#include "noiseSimplex.hlsli"
#include "math.hlsli"

// NOISE SETTINGS STRUCT DEFINITIONS

struct SimpleNoiseSettings
{
    float elevation;
    float frequency;
    float verticalShift;
    int octaves;
    float2 offset;
    float persistence;
    float lacunarity;
};

struct RidgeNoiseSettings
{
    float elevation;
    float frequency;
    int octaves;
    float persistence;
    float lacunarity;
    float2 offset;
    float power;
    float gain;
    float ridgeThreshold;
    
    float2 padding;
};

struct MountainNoiseSettings
{
    float elevation;
    float frequency;
    int octaves;
    float persistence;
    
    float lacunarity;
    float2 offset;
    float blending;
    
    float detail;
    
    float3 padding;
};

struct TerrainNoiseSettings
{
    SimpleNoiseSettings continentSettings;
    MountainNoiseSettings mountainSettings;
    RidgeNoiseSettings ridgeSettings;
    
    float oceanDepthMultiplier;
    float oceanFloorDepth;
    float oceanFloorSmoothing;
    float padding;
};


// NOISE FUNCTIONS

float SimpleNoise(float2 pos, SimpleNoiseSettings settings)
{
    float noiseSum = 0.0f;
    float f = settings.frequency;
    float a = 1.0f;
    
    for (int octave = 0; octave < settings.octaves; octave++)
    {
        noiseSum += snoise(f * pos + settings.offset) * a;
        
        f *= settings.lacunarity;
        a *= settings.persistence;
    }
    
    return noiseSum * settings.elevation + settings.verticalShift;
}


float RidgeNoise(float2 pos, RidgeNoiseSettings settings)
{
    float noiseSum = 0.0f;
    float f = settings.frequency;
    float a = 1.0f;
    float ridgeWeight = 1.0f;
    
    for (int octave = 0; octave < settings.octaves; octave++)
    {
        float noiseVal = 1.0f - abs(snoise(f * pos + settings.offset));
        noiseVal = pow(abs(noiseVal), settings.power);
        noiseVal *= ridgeWeight;
        ridgeWeight = saturate(noiseVal * settings.gain);
        
        noiseSum += noiseVal * a;
        
        f *= settings.lacunarity;
        a *= settings.persistence;
    }
    
    return noiseSum * settings.elevation;
}


float MountainNoise(float2 pos, MountainNoiseSettings settings)
{
    float noiseSum = 0.0f;
    float f = settings.frequency;
    float a = 1.0f;
    float ridgeWeight = 1.0f;
    
    for (int octave = 0; octave < settings.octaves; octave++)
    {
        float noiseVal1 = abs(snoise(f * pos + settings.offset));
        float noiseVal2 = noiseVal1 * (1.0f - smoothstep(0, noiseVal1, settings.blending));
        
        noiseSum += noiseVal2 * a;
        
        f *= settings.lacunarity;
        a *= settings.persistence * (1.0f - smoothstep(0, noiseVal1, settings.detail));
    }
    
    return noiseSum * settings.elevation;
}

float TerrainNoise(float2 pos, TerrainNoiseSettings terrainSettings)
{
    // create continent shape
    float continentShape = SimpleNoise(pos, terrainSettings.continentSettings);
    
    // create mountains
    float mountainShape = 0, ridgeShape = 0;
    if (terrainSettings.mountainSettings.elevation > 0.0f)
    {
        mountainShape = MountainNoise(pos, terrainSettings.mountainSettings);
    }
    // create ridges
    if (terrainSettings.ridgeSettings.elevation > 0.0f)
    {
        ridgeShape = RidgeNoise(pos, terrainSettings.ridgeSettings);
        ridgeShape *= smoothstep(0, terrainSettings.ridgeSettings.ridgeThreshold, mountainShape);
    }
    
    // create ocean floor
    continentShape = smoothMax(continentShape, -terrainSettings.oceanFloorDepth, terrainSettings.oceanFloorSmoothing);
    if (continentShape < 0)
        continentShape *= 1 + terrainSettings.oceanDepthMultiplier;
    
    return continentShape + mountainShape + ridgeShape;
}