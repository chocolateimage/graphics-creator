#include <iostream>

extern "C" {
__declspec(dllexport) void b01() { std::cout << "b01" << std::endl; }
__declspec(dllexport) void b02() { std::cout << "b02" << std::endl; }
__declspec(dllexport) void b03() { std::cout << "b03" << std::endl; }
__declspec(dllexport) void b04() { std::cout << "b04" << std::endl; }
}
