// Smooth minimum of two values, controlled by smoothing factor k
// When k = 0, this behaves identically to min(a, b)
float smoothMin(float a, float b, float k)
{
    k = max(0, k);
    // https://www.iquilezles.org/www/articles/smin/smin.htm
    float h = max(0, min(1, (b - a + k) / (2 * k)));
    return a * h + b * (1 - h) - k * h * (1 - h);
}

// Smooth maximum of two values, controlled by smoothing factor k
// When k = 0, this behaves identically to max(a, b)
float smoothMax(float a, float b, float k)
{
    k = min(0, -k);
    float h = max(0, min(1, (b - a + k) / (2 * k)));
    return a * h + b * (1 - h) - k * h * (1 - h);
}


// taken from: https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d

// adapted from intersectCube in https://github.com/evanw/webgl-path-tracing/blob/master/webgl-path-tracing.js

// compute the near and far intersections of the cube (stored in the x and y components) using the slab method
// no intersection means vec.x > vec.y (really tNear > tFar)
float2 intersectAABB(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax)
{
    float3 tMin = (boxMin - rayOrigin) / rayDir;
    float3 tMax = (boxMax - rayOrigin) / rayDir;
    float3 t1 = min(tMin, tMax);
    float3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return float2(tNear, tFar);
};



float remap01(float v)
{
    return saturate(0.5f * (1.0f + v));
}
float remap01(float v, float v_min, float v_max)
{
    return saturate((v - v_min) / (v_max - v_min));
}
float2 remap01(float2 v)
{
    return saturate(0.5f * (1.0f + v));
}
float3 remap01(float3 v)
{
    return saturate(0.5f * (1.0f + v));
}
float4 remap01(float4 v)
{
    return saturate(0.5f * (1.0f + v));
}
