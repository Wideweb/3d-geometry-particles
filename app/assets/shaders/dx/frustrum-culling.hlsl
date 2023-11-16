struct InstanceData
{
    float3 min;
    float padding0;
    float3 max;
    float padding1;
};

cbuffer cbCommon : register(b0)
{
	float3 frustrumMin;
    float padding0;
    float3 frustrumMax;
    float padding1;
};

StructuredBuffer<InstanceData> instances : register(t0);
RWStructuredBuffer<uint> visible : register(u0);

bool CheckFrustrumCulling(float3 min, float3 max)
{
    return max.x > frustrumMin.x && min.x < frustrumMax.x 
        && max.y > frustrumMin.y && min.y < frustrumMax.y
        && max.z > frustrumMin.z && min.z < frustrumMax.z;
}

[numthreads(256, 1, 1)]
void CS(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint index = dispatchThreadId.x;

    InstanceData instance = instances[index];

    if (CheckFrustrumCulling(instance.min, instance.max))
    {
        uint visibleIndex;
        InterlockedAdd(visible[0], 1, visibleIndex);
        visible[visibleIndex + 1] = index;
    }
}