#include "shared_lib.hpp"
#include <iostream>

extern "C" {
__declspec(dllexport) void a01() { std::cout << "a01" << std::endl; }
__declspec(dllexport) void a02() { std::cout << "a02" << std::endl; }
__declspec(dllexport) void a03() { std::cout << "a03" << std::endl; }
__declspec(dllexport) void a04() { std::cout << "a04" << std::endl; }
__declspec(dllexport) void a05() { std::cout << "a05" << std::endl; }
__declspec(dllexport) void a06() { std::cout << "a06" << std::endl; }
__declspec(dllexport) void a07() { std::cout << "a07" << std::endl; }
__declspec(dllexport) void a08() { std::cout << "a08" << std::endl; }
__declspec(dllexport) void a09() { std::cout << "a09" << std::endl; }
__declspec(dllexport) void a10() { std::cout << "a10" << std::endl; }
__declspec(dllexport) void a11() { std::cout << "a11" << std::endl; }
__declspec(dllexport) void a12() { std::cout << "a12" << std::endl; }
__declspec(dllexport) void a13() { std::cout << "a13" << std::endl; }
__declspec(dllexport) void a14() { std::cout << "a14" << std::endl; }
__declspec(dllexport) void a15() { std::cout << "a15" << std::endl; }
__declspec(dllexport) void *a16(int count, int *param) {
    std::cout << "a16 (allocating " << count << " bytes)" << std::endl;
    void *allocated = malloc(count);
    return allocated;
}
__declspec(dllexport) void a17() { std::cout << "a17" << std::endl; }
__declspec(dllexport) void a18() { std::cout << "a18" << std::endl; }
__declspec(dllexport) void a19() { std::cout << "a19" << std::endl; }
__declspec(dllexport) void a20() { std::cout << "a20" << std::endl; }
__declspec(dllexport) void a21() { std::cout << "a21" << std::endl; }
__declspec(dllexport) void a22() { std::cout << "a22" << std::endl; }
__declspec(dllexport) void a23() { std::cout << "a23" << std::endl; }
__declspec(dllexport) void a24() { std::cout << "a24" << std::endl; }
__declspec(dllexport) void a25() { std::cout << "a25" << std::endl; }
__declspec(dllexport) void a26() { std::cout << "a26" << std::endl; }
__declspec(dllexport) void a27() { std::cout << "a27" << std::endl; }
__declspec(dllexport) void a28() { std::cout << "a28" << std::endl; }
__declspec(dllexport) void a29() { std::cout << "a29" << std::endl; }
__declspec(dllexport) void a30() { std::cout << "a30" << std::endl; }
__declspec(dllexport) void a31() { std::cout << "a31" << std::endl; }
__declspec(dllexport) void a32() { std::cout << "a32" << std::endl; }
__declspec(dllexport) void a33() { std::cout << "a33" << std::endl; }
__declspec(dllexport) void a34() { std::cout << "a34" << std::endl; }
__declspec(dllexport) void a35() { std::cout << "a35" << std::endl; }
__declspec(dllexport) void a36() { std::cout << "a36" << std::endl; }
__declspec(dllexport) void a37() { std::cout << "a37" << std::endl; }
__declspec(dllexport) void a38() { std::cout << "a38" << std::endl; }
__declspec(dllexport) void a39() { std::cout << "a39" << std::endl; }
__declspec(dllexport) void a40() { std::cout << "a40" << std::endl; }
__declspec(dllexport) void *a41() {
    std::cout << "a41" << std::endl;
    return nullptr;
}
__declspec(dllexport) void a42() { std::cout << "a42" << std::endl; }
__declspec(dllexport) MsvcString *a43(void *first, MsvcString *a,
                                      MsvcString *b) {
    std::string abc;
    std::cout << "a43 (DebugDatabase GetEntry)" << std::endl;
    std::cout << "string: " << b->data() << std::endl;
    a->set("4");
    return a;
}
}
