#include <format>
#include <optional>
#include <string>
#include <utility>

#include <cstddef>

#include <dlfcn.h>

#include "pvk/dso_loader.hh"

#include "log.hh"

namespace {
static void log_preload_error(const std::string &msg)
try {
    log::warning(std::format("[DEBUG]: DSO Preload error: \"{}\"\n", msg));
} catch (...) {
}

static void log_unload_failue(const std::string &lib_name)
try {
    log::warning(std::format("Unload library {} failue", lib_name));
} catch (...) {
}

} // namespace

namespace pvk {

std::optional<SymLoader> SymLoader::load(const std::string &library_file)
{
    char *preload_error = dlerror();
    if (preload_error != NULL) {
        log_preload_error(preload_error);
    }
    void *new_handle = dlopen(library_file.c_str(), RTLD_NOW);
    char *load_status = dlerror();
    if (load_status != NULL) {
        log::error(std::format(
            "Error loading library from: \"{}\" reason \"{}\"\n",
            library_file,
            load_status
        ));
        return std::nullopt;
    }

    SymLoader output;
    output.m_handle = new_handle;
    output.m_library_path = library_file;
    return output;
}

SymLoader::SymLoader() = default;

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
        log_unload_failue(m_library_path);
    }
}
} // namespace pvk
