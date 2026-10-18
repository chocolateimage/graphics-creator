#include <iostream>

extern "C" {
__declspec(dllexport) void om01() { std::cout << "om01" << std::endl; }
}
