//! The demo functions that get to be patched
#pragma once

#include "runtime_patching_lib.h"

class DemoClass {
public:
    FORCE_NO_INLINE void say_hello(int number, std::string_view str);
};

void DemoClass::say_hello(int number, std::string_view str) {
    std::cout << "Hello, " << str << "! " << number << std::endl;
}

FORCE_NO_INLINE void say_hello_fun(int number, std::string_view str) {
    std::cout << "Hello, " << str << "! " << number << std::endl;
}
