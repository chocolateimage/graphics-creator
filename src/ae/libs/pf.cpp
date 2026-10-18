#include <iostream>

extern "C" {
__declspec(dllexport) void PFp_G_fcd_table() {
    std::cout << "PFp_G_fcd_table" << std::endl;
}
__declspec(dllexport) void PF_AccumulateUnoBuffers() {
    std::cout << "PF_AccumulateUnoBuffers" << std::endl;
}
__declspec(dllexport) void PF_FillDeepUnoBuffer() {
    std::cout << "PF_FillDeepUnoBuffer" << std::endl;
}
__declspec(dllexport) void PF_FillUnoBuffer() {
    std::cout << "PF_FillUnoBuffer" << std::endl;
}
__declspec(dllexport) void PF_InvertAndFadeUnoBuffer() {
    std::cout << "PF_InvertAndFadeUnoBuffer" << std::endl;
}
__declspec(dllexport) void PF_TransformWorld() {
    std::cout << "PF_TransformWorld" << std::endl;
}
__declspec(dllexport) void pf01() { std::cout << "pf01" << std::endl; }
__declspec(dllexport) void pf02() { std::cout << "pf02" << std::endl; }
__declspec(dllexport) void pf03() { std::cout << "pf03" << std::endl; }
__declspec(dllexport) void pf04() { std::cout << "pf04" << std::endl; }
__declspec(dllexport) void pf05() { std::cout << "pf05" << std::endl; }
}
