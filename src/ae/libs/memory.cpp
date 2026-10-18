#include <iostream>

extern "C" {
__declspec(dllexport) void PrDisposePtr() {
    std::cout << "PrDisposePtr" << std::endl;
}
}
