StructuredBuffer<float> buffer0;
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    uint threadIndex = threadId.x;
    result[threadIndex] = buffer0[threadIndex] + buffer1[threadIndex];
}
