#include <format>
#include <optional>
#include <string>
#include <utility>

#include <cstddef>

#include <Windows.h>

#include "pvk/dso_loader.hh"

#include "log.hh"

namespace {
/*
 *  +---------------+
 *  | Symmetry stub |
 *  +---------------+
 */

static void log_unload_failue(const std::string &lib_name)
try {
    pvk::warning(std::format("Unload library {} failue", lib_name));
} catch (...) {
}

} // namespace

namespace pvk {

std::optional<SymLoader> SymLoader::load(const std::string &library_file)
{
    void *new_handle = LoadLibraryA(library_file.c_str());
    DWORD last_error_code = GetLastError();
    if (new_handle == NULL) {
        pvk::warning(std::format(
            "Failue loading library from: \"{}\" error code 0x{:X}",
            library_file,
            last_error_code
        ));
        return std::nullopt;
    }

    SymLoader output;
    output.m_handle = new_handle;
    output.m_library_path = library_file;
    return output;
}

/*  +---------------+
 *  | Symmetry stub |
 *  +---------------+ */

SymLoader::SymLoader() = default;

SymLoader::SymLoader(SymLoader &&other) noexcept
{
    m_library_path = std::move(other.m_library_path);
    std::swap(m_handle, other.m_handle);
}

std::optional<void *> SymLoader::load_sym(const std::string &symname)
{
    auto *new_sym =
        GetProcAddress(reinterpret_cast<HMODULE>(m_handle), symname.c_str());
    if (new_sym == NULL) {
        return std::nullopt;
    }
    return new_sym;
}

/*
 *  +---------------+
 *  | Symmetry stub |
 *  +---------------+
 */

SymLoader::~SymLoader() noexcept
{
    if (m_handle == NULL) {
        return;
    }

    int unload_status = FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
    if (unload_status == 0) {
        log_unload_failue(m_library_path);
    }
}
} // namespace pvk
