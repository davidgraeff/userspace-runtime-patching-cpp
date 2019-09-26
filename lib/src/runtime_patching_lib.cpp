#include "runtime_patching_lib.h"
#include "vendor/json.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using namespace std::literals;
using namespace nlohmann;

auto PatchRegistry::get_patch_directory() -> Result<PatchRegistry::cache_pointer> {
    auto now = std::chrono::system_clock::now();
    auto diff = now - this->cache_time;

    // If the cache is expired
    if (diff > 1h) {
        this->cache_time = now;
        // Load registry meta data
        fs::path registry_url = fs::current_path() / this->registry_uri;
        std::ifstream i(registry_url);
        string str;
        if (i) {
            std::ostringstream ss;
            ss << i.rdbuf();
            str = ss.str();
        } else {
            std::cerr << "Did not find " << registry_url << "\n";
            return Result<PatchRegistry::cache_pointer>("File not found");
        }

        // Parse json
        auto j = json::parse(str);
        for (auto &element : j) {
            cache.emplace_back(Patch{
                    .new_version = element["new_version"].get<int>(),
                    .about = element["about"].get<std::string>(),
                    .symbol_name = element["symbol_name"].get<std::string>(),
                    .patch_file = element["patch_file"].get<std::string>(),
            });
        }
    }

    return Result<PatchRegistry::cache_pointer>(&cache);
}

PatchRegistry::PatchRegistry(std::string registry_uri) noexcept : registry_uri(std::move(registry_uri)) {
}

#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <utility>

#include "lde_minimal.h"
#include "make_jmp.h"

#define NOP_OPCODE  0x90

void perform_patch(const Patch &patch, Patchable &patchable) {
    fs::path patch_file = fs::current_path() / patch.patch_file;
    auto handle = dlopen(patch_file.c_str(), RTLD_NOW);
    if (!handle) {
        std::cerr << "Failed to load shared library " << patch.patch_file << "!\n" << dlerror() << "\n";
        return;
    }
    auto patched_function = dlsym(handle, patch.symbol_name.c_str());
    if (!patched_function) {
        std::cerr << "dlsym failed. Did not find " << patch.symbol_name << "!\n";
        return;
    }

    // Align pointer to page size
    void *page = (void *) (uintptr_t(patchable.address) & ~(getpagesize() - 1));
    size_t min_size = get_jmp_size(patchable.address, patched_function);
    size_t actual_size = disasm_until(patchable.address, min_size);

    if (!actual_size) {
        std::cerr << "disasm_until failed. Address invalid " << patchable.address << "!\n";
        return;
    }

    // Make the page with the target function writable
    if (mprotect(page, actual_size, PROT_WRITE | PROT_READ | PROT_EXEC))
        perror("Please disable seccomp, SELinux, AppArmor. mprotect call failed!");

    // The easy part: It is a vtable change
    if (patchable.vtable_index >= 0) {
        auto a = static_cast<intptr_t *>(patchable.address) + patchable.vtable_index;
        *a = (intptr_t) patched_function;
    } else {
        // The stack including the return address and ECX register (for c++ member functions)
        // are already prepared. We just want to jump. Write the jmp instruction right at the front of the target functions address.
        // This does not account for too-small target functions. Those are expected to be at least as big as the used jmp.
        make_jmp(patchable.address, patched_function);
        // Fill with NOPs
        for (size_t addr = min_size; addr < actual_size; ++addr) {
            *((uint8_t *) patchable.address + addr) = NOP_OPCODE;
        }
    }

    // Make the page with the target function non-writable. VirtualProtect on Windows.
    if (mprotect(page, getpagesize(), PROT_READ | PROT_EXEC))
        perror("Please disable seccomp, SELinux, AppArmor. mprotect call failed!");

    patchable.current_version = patch.new_version;
    std::clog << "Patched " << patch.symbol_name << " to " << patch.new_version << "\n";
}

void patch_now(Patchables &patchables, PatchRegistry &patch_registry) {
    auto cache_result = patch_registry.get_patch_directory();
    auto cache = std::get_if<PatchRegistry::cache_pointer>(&cache_result);
    if (!cache) {
        std::cerr << "Failed to get registry cache pointer!\n";
        return;
    }

    for (auto &cache_entry: **cache) {
        for (auto &patchable: patchables) {
            if (cache_entry.symbol_name == patchable.symbol_name) {
                if (cache_entry.new_version <= patchable.current_version) {
                    std::clog << "Not patching " << cache_entry.symbol_name << ". Already up to date\n";
                    continue;
                }
                std::clog << "Patching " << cache_entry.symbol_name << " to " << cache_entry.new_version << "\n";
                perform_patch(cache_entry, patchable);
            }
        }
    }
}
