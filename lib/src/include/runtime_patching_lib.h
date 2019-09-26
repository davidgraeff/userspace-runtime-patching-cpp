#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <future>
#include <optional>
#include <variant>

using string = std::string;

// Unfortunately still not unified.
// We need to force target functions to not be inlined, ever.
#ifdef WIN32
#define FORCE_NO_INLINE __declspec(noinline)
#else
#define FORCE_NO_INLINE __attribute__((noinline))
#endif

/// This library does not use exceptions, but std::variant to inform about either
/// a successful call or an error. The error type is a string
template<class T>
using Result = std::variant<T, std::string_view>;

/// A single patch. A patch consists of the target symbol, a version and a description.
struct Patch {
    /// The patch version. A patch only happens if this number is bigger than #current_version.
    int new_version=0;
    /// A description of this patch. Shown to the user after the registry meta data has been refreshed.
    string about;
    /// The symbol name, that will be used for `dlsym` to find the symbols name.
    string symbol_name;
    /// The path to the binary fragment. This is expected to be a shared library (.so) filesystem URI in this implementation.
    /// It is task of the ::PatchRegistry to resolve non file system URIs to filesystem URIs.
    string patch_file;
};

/// The patch registry class. Contains a cached list of available patches.
class PatchRegistry {
private:
    std::vector<Patch> cache;
    std::chrono::system_clock::time_point cache_time;
    std::string registry_uri;
public:
    explicit PatchRegistry(std::string registry_uri) noexcept ;

    using cache_pointer = std::vector<Patch>*;
    /// This method returns the patch registry entries. Entries are cached. If the cache is older than 60 minutes
    /// it will be refreshed first.
    auto get_patch_directory() -> Result<cache_pointer>;
};

struct Patchable {
    /// The patchable functions pointer address. Vtable pointer for virtual member functions of classes.
    void* address;
    /// The vtable index for virtual member functions, ignored otherwise. Can be obtained in a non standard way via for example &MyClass::my_method
    int vtable_index = -1;
    /// The current functions version. Is 0 by default.
    int current_version=0;
    /// The symbol name. Obtain it in a non standard way via typeid(&MyClass::my_method).name()
    string symbol_name;
};

using Patchables = std::vector<Patchable>;

/// Patches all patchables if a matching entry in the patch registry could be found.
void patch_now(Patchables& patchables, PatchRegistry& patch_registry);

/// Determines the patchable address of a C++ class member function.
/// Do not use this on virtual class members!
///
/// Example usage: auto addr = cpp_class_member_address(&DemoClass::say_hello);
/// \tparam T2 The class member type
/// \param v A reference to the class member
/// \return The address
template<class T2>
inline void *cpp_class_member_address(T2 v) {
    union UT {
        void *t1;
        T2 t2;
    } u{};
    u.t2 = v;
    return u.t1;
}
