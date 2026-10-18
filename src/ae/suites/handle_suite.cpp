#include "handle_suite.hpp"
#include "ae/utility_callbacks.hpp"

PFHandleSuite2::PFHandleSuite2() {
    createMemory = ::createMemory;
    useMemory = ::useMemory;
    freeMemoryUsage = ::freeMemoryUsage;
    freeMemory = ::freeMemory;
}
