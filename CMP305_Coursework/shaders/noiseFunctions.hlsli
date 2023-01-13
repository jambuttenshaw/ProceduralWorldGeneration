#include "noiseSimplex.hlsli"
#include "math.hlsli"

// NOISE SETTINGS STRUCT DEFINITIONS

struct SimpleNoiseSettings
{
    // 16 bytes
    float elevation;
    float frequency;
    float verticalShift;
    int octaves;
    // 16 bytes
    float2 offset;
    float persistence;
    float lacunarity;
};

struct RidgeNoiseSettings
{
    // 16 bytes
    float elevation;
    float frequency;
    float verticalShift;
    int octaves;
    // 16 bytes
    float2 offset;
    float persistence;
    float lacunarity;
    // 16 bytes
    float power;
    float gain;
    float peakSmoothing;
    float padding0;
};

struct TerrainNoiseSettings
{
    SimpleNoiseSettings continentSettings;
    RidgeNoiseSettings mountainSettings;
    
    float oceanDepthMultiplier;
    float oceanFloorDepth;
    float oceanFloorSmoothing;
    float mountainBlend;
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
    
    return noiseSum * settings.elevation + settings.verticalShift;
}


float SmoothedRidgeNoise(float2 pos, RidgeNoiseSettings settings)
{
    float offset = settings.peakSmoothing * 0.01f;
    
    float sample0 = RidgeNoise(pos, settings);
    float sample1 = RidgeNoise(pos + float2(offset, 0.0f), settings);
    float sample2 = RidgeNoise(pos + float2(0.0f, offset), settings);
    float sample3 = RidgeNoise(pos - float2(offset, 0.0f), settings);
    float sample4 = RidgeNoise(pos - float2(0.0f, offset), settings);
    float sample5 = RidgeNoise(pos + float2(offset, offset), settings);
    float sample6 = RidgeNoise(pos + float2(-offset, offset), settings);
    float sample7 = RidgeNoise(pos - float2(offset, offset), settings);
    float sample8 = RidgeNoise(pos - float2(-offset, offset), settings);
    
    return (sample0 + sample1 + sample2 + sample3 + sample4 + sample5 + sample6 + sample7 + sample8) / 9.0f;
}

float NewRidgeNoise(float2 pos, RidgeNoiseSettings settings)
{
    float noiseSum = 0.0f;
    float f = settings.frequency;
    float a = 1.0f;
    float ridgeWeight = 1.0f;
    
    for (int octave = 0; octave < settings.octaves; octave++)
    {
        float noiseVal1 = abs(snoise(f * pos + settings.offset));
        float noiseVal2 = noiseVal1 * (1.0f - smoothstep(0, noiseVal1, settings.gain));
        
        noiseSum += noiseVal2 * a;
        
        f *= settings.lacunarity;
        a *= settings.persistence * (1.0f - smoothstep(0, noiseVal1, settings.peakSmoothing));
    }
    
    return noiseSum * settings.elevation + settings.verticalShift;
}

float TerrainNoise(float2 pos, TerrainNoiseSettings terrainSettings)
{
    // create continent shape
    float continentShape = SimpleNoise(pos, terrainSettings.continentSettings);
    // create mountains
    float mountainShape = NewRidgeNoise(pos, terrainSettings.mountainSettings);
    
    // apply ocean floor
    continentShape = smoothMax(continentShape, -terrainSettings.oceanFloorDepth, terrainSettings.oceanFloorSmoothing);
    if (continentShape < 0)
        continentShape *= 1 + terrainSettings.oceanDepthMultiplier;
    
    return continentShape + mountainShape;
}