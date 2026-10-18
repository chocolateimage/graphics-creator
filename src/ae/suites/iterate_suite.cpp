#include "iterate_suite.hpp"
#include "ae/utility_callbacks.hpp"

PFIterate8Suite1::PFIterate8Suite1() {
    iterate = ::iterate;
    iterateOrigin = ::iterateOrigin;
}

PFIterate8Suite2::PFIterate8Suite2() {
    iterate = ::iterate;
    iterateOrigin = ::iterateOrigin;
}

void unsupported() {
    qInfo() << "OTHER ITERATION SUITES NOT IMPLEMENTED!";
    abort();
}

PFIterate16Suite2::PFIterate16Suite2() {
    func1 = (void *)::unsupported;
    func2 = (void *)::unsupported;
    func3 = (void *)::unsupported;
}

PFIterateFloatSuite2::PFIterateFloatSuite2() {
    func1 = (void *)::unsupported;
    func2 = (void *)::unsupported;
    func3 = (void *)::unsupported;
}
