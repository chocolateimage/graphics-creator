#include <iostream>

extern "C" {
__declspec(dllexport) void MSK_MaskWithShape() {
    std::cout << "MSK_MaskWithShape" << std::endl;
}
}
