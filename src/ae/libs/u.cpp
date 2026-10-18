#include <iostream>

extern "C" {
__declspec(dllexport) void d01() { std::cout << "d01" << std::endl; }
__declspec(dllexport) void d02() { std::cout << "d02" << std::endl; }
__declspec(dllexport) void d03() { std::cout << "d03" << std::endl; }
__declspec(dllexport) void d04() { std::cout << "d04" << std::endl; }
__declspec(dllexport) void d05() { std::cout << "d05" << std::endl; }
__declspec(dllexport) void d06() { std::cout << "d06" << std::endl; }
__declspec(dllexport) void d07() { std::cout << "d07" << std::endl; }
__declspec(dllexport) void d08() { std::cout << "d08" << std::endl; }
__declspec(dllexport) void M_UnionLRect() {
    std::cout << "M_UnionLRect" << std::endl;
}
__declspec(dllexport) void M_RoundDouble() {
    std::cout << "M_RoundDouble" << std::endl;
}
__declspec(dllexport) void M_ReduceRatio() {
    std::cout << "M_ReduceRatio" << std::endl;
}
}
