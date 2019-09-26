#include <string_view>
#include <iostream>

using namespace std;

class DemoClass {
public:
    void say_hello(int number, string_view str);
};

void DemoClass::say_hello(int number, string_view str) {
    std::cout << "Completely patched! " << str << "! " << number << std::endl;
}

void say_hello_fun(int number, string_view str) {
    std::cout << "Completely patched! " << str << "! " << number << std::endl;
}