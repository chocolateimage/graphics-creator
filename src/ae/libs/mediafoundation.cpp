#include <iostream>

extern "C" {
__declspec(dllexport) void g01() { std::cout << "g01" << std::endl; }
__declspec(dllexport) void g02() { std::cout << "g02" << std::endl; }
__declspec(dllexport) void g03() { std::cout << "g03" << std::endl; }
}
