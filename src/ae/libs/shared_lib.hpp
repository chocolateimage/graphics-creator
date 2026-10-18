#pragma once
#include <string>

struct MsvcString {
    union {
        char *ptr;
        char buf[16];
    };
    size_t size;
    size_t capacity;

    bool is_large() { return capacity > 15; }
    auto data() { return is_large() ? ptr : buf; }

    void clear() {
        if (is_large()) {
            free(ptr);
            ptr = nullptr;
        }
        size = 0;
        capacity = 0;
    }
    void set(const std::string &string) {
        // clear();
        size = string.size();
        capacity = string.size();
        if (is_large()) {
            ptr = (char *)malloc(string.size() + 1);
            strcpy(ptr, string.c_str());
        } else {
            strcpy_s(buf, string.c_str());
        }
    }
};
