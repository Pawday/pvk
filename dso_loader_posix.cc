#include <cstddef>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include <dlfcn.h>

#include "pvk/dso_loader.hh"

namespace {
static void log_preload_error(const std::string &msg)
{
    std::cerr << std::format("[DEBUG]: DSO Preload error: \"{}\"\n", msg);
}
} // namespace

std::optional<SymLoader> SymLoader::load(const std::string &library_file)
{
    char *preload_error = dlerror();
    if (preload_error != NULL) {
        log_preload_error(preload_error);
    }
    void *new_handle = dlopen(library_file.c_str(), RTLD_NOW);
    char *load_status = dlerror();
    if (load_status != NULL) {
        std::cerr << std::format(
            "Error loading library from: \"{}\" reason \"{}\"\n",
            library_file,
            load_status
        );
        return std::nullopt;
    }

    SymLoader output;
    output.m_handle = new_handle;
    output.m_library_path = library_file;
    return output;
}

SymLoader::SymLoader()
{
}

SymLoader::SymLoader(SymLoader &&other) noexcept
{
    m_library_path = std::move(other.m_library_path);

    m_handle = other.m_handle;
    other.m_handle = NULL;
    other.m_moved = true;
}

std::optional<void *> SymLoader::load_sym(const std::string &symname)
{
    char *preload_error = dlerror();
    if (preload_error != NULL) {
        log_preload_error(preload_error);
    }

    void *new_sym = dlsym(m_handle, symname.c_str());
    char *symload_status = dlerror();
    if (symload_status != NULL) {
        return std::nullopt;
    }

    return new_sym;
}

SymLoader::~SymLoader() noexcept
{
    if (m_moved) {
        return;
    }

    int unload_status = dlclose(m_handle);
    if (unload_status != 0) {
        try {
            std::cerr << std::format(
                "Unload library {} faild - just saying\n", m_library_path
            );
        } catch (...) {
            // Just log, nothing bad, can ignore
        }
    }
}
