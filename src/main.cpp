#include <iostream>
#include <future>
#include <functional>
#include <climits>

#include "runtime_patching_lib.h"
#include "read_from_input.h"
#include "demo_functions.h"
#include <cxxabi.h>

using namespace std;

int main() {
    init_term();

    DemoClass demo;
    // Bind to function pointer
    auto say_hello = bind(&DemoClass::say_hello, &demo, 42, "from C++ member function");
    auto say_hello_fun_bind = bind(&say_hello_fun, 42, "from C function");

    PatchRegistry registry("registry/meta.json");
    Patchables patchables;
    auto v = cpp_class_member_address(&DemoClass::say_hello);
    std::clog << v << "\n" << &demo << "\n" << typeid(&DemoClass::say_hello).name() << "\n";

    int     status;
    char* realname = abi::__cxa_demangle(typeid(&DemoClass::say_hello).name(), 0, 0, &status);
    char* realname2 = abi::__cxa_demangle("_ZN9DemoClass9say_helloEiSt17basic_string_viewIcSt11char_traitsIcEE", 0, 0, &status);
    std::clog << v << "\n" << &demo << "\n" << typeid(&DemoClass::say_hello).name() << " " << realname  << " " << realname2<< "\n";

    patchables.emplace_back(Patchable{
            .address=cpp_class_member_address(&DemoClass::say_hello),
            .symbol_name="_ZN9DemoClass9say_helloEiSt17basic_string_viewIcSt11char_traitsIcEE"
    });

    patchables.emplace_back(Patchable{
            .address=reinterpret_cast<void *>( &say_hello_fun),
            .symbol_name="_Z13say_hello_funiSt17basic_string_viewIcSt11char_traitsIcEE"
    });

    std::cout << "Press u for updating the registry. Press p for patching. Press c for canceling.\n";
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Current working dir: " << cwd << "\n";
    }
    std::cout.flush();    // ensure output is written

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        // Calling our function, that is soon to be patched
        say_hello();
        say_hello_fun_bind();

        auto c = getch_async();

        switch (c) {
            case EOF: {
                continue;
            }
            case 'u': {
                std::cout << "Updating now" << "\n";
                auto result = registry.get_patch_directory();
                auto cache = std::get_if<PatchRegistry::cache_pointer>(&result);
                if (!cache) {
                    std::cerr << "Failed to get registry cache pointer!\n";
                    continue;
                }
                for (auto &cache_entry: **cache) {
                    std::clog << "Found patch for " << cache_entry.symbol_name << " (version: "
                              << cache_entry.new_version
                              << "):" << cache_entry.about << "\n";
                }
                break;
            }
            case 'p': {
                std::cout << "Patching now" << "\n";
                patch_now(patchables, registry);
                break;
            }
            case 'c': {
                return 0;
            }
            default:
                continue;
        }
    }
#pragma clang diagnostic pop
    restore_term();
    return 0;
}