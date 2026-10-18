#include <iostream>

extern "C" {
__declspec(dllexport) void csc01() { std::cout << "csc01" << std::endl; }
__declspec(dllexport) void csc02() { std::cout << "csc02" << std::endl; }
__declspec(dllexport) void csc03() { std::cout << "csc03" << std::endl; }
}
