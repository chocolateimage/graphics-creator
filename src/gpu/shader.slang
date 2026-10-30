[format("rgba8")] RWTexture2D texture;
[format("rgba8")] Sampler2D inputTexture;
struct Params {
    float radius;
};
[[vk::push_constant]] Params pc;

[shader("compute")]
[numthreads(8,8,1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    uint x = threadId.x;
    uint y = threadId.y;

    uint2 size;
    texture.GetDimensions(size.x, size.y);

    if (x >= size.x || y >= size.y) return;
    // float firstValue = buffer0[threadIndex];
    // firstValue = fmod(firstValue, 600.0);
    // firstValue = firstValue * firstValue * firstValue;
    // firstValue = abs(firstValue);
    // firstValue = fmod(firstValue, 30.0);
    // firstValue = round(firstValue);

    // float secondValue = buffer1[threadIndex];
    // secondValue = fmod(secondValue, 800.0);
    // secondValue = pow(secondValue, 6);
    // secondValue = abs(secondValue);
    // secondValue = fmod(secondValue, 60.0);
    // secondValue = round(secondValue);

    // result[threadIndex] = (firstValue / 30.) +secondValue / 60.;
    // int x = threadIndex % 1920;
    // int y = threadIndex / 1920;
    // result[threadIndex] = (x / 1920.0) + (y / 1080.0);


    float2 uv = (float2(x, y) + 0.5) / float2(size);
    float4 final = 0;
    float count = 0;
    // int bluriness = (int)(((float)x / (float)size.x) * 32);
    float widx = ((float)x)/((float)size.x);
    int bluriness = max(1,(int)(pc.radius*widx));
    float sigma = bluriness / 3.;
    for (int sy = -bluriness; sy <= bluriness; sy++) {
        for (int sx = -bluriness; sx <= bluriness; sx++) {
            float amount = exp(-(sx*sx+sy*sy)/(2*sigma*sigma)) / (2*3.141*sigma*sigma);
            // float amount = distance(float2(sx,sy), float2(0,0))/16;
            // float2 uv = (float2(x + sx, y + sy) + 0.5) / float2(size);
            final += inputTexture.SampleLevel(float2(x+sx,y+sy), 0.0)*amount;
            count += amount;
        }
    }
    texture[uint2(x,y)] = final / count;
}
