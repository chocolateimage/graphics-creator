#include "aegp_memory_suite.hpp"
#include "ae/ae_base.hpp"

int32_t newMemoryHandle(int32_t pluginId, char *str, uint32_t size, int32_t,
                        AEMemoryHandle **handle) {
    qInfo() << "create memory" << str << "with" << size << "bytes";
    AEMemoryHandle *mem = new AEMemoryHandle();
    mem->size = size;
    mem->data = malloc(size);
    memset(mem->data, 0, size);
    *handle = mem;

    return 0;
}

AEGPMemorySuite::AEGPMemorySuite() {
    newMemoryHandle = (void *)::newMemoryHandle;
}
