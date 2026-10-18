#include "ae_base.hpp"

AEMemoryHandle::AEMemoryHandle() {}
AEMemoryHandle *AEMemoryHandle::copy() {
    AEMemoryHandle *handle = new AEMemoryHandle();
    handle->size = size;
    handle->data = malloc(size);
    memcpy(handle->data, data, size);
    return handle;
}
AEMemoryHandle::~AEMemoryHandle() { free(data); }
