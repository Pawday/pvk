#include <cstddef>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include <Windows.h>

#include "pvk/dso_loader.hh"

namespace {
static void log_preload_error(const std::string &msg)
{
    std::cerr << std::format("DSO Preload error: \"{}\"\n", msg);
}
} // namespace

std::optional<SymLoader> SymLoader::load(const std::string &library_file)
{
    void *new_handle = LoadLibraryA(library_file.c_str());
    if (new_handle == NULL) {
        std::cerr << std::format(
            "Error loading library from: \"{}\" reason \"Microsoft\"\n",
            library_file
        );
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
    auto *new_sym =
        GetProcAddress(reinterpret_cast<HMODULE>(m_handle), symname.c_str());
    if (new_sym == NULL) {
        return std::nullopt;
    }
    return new_sym;
}

SymLoader::~SymLoader() noexcept
{
    if (m_moved) {
        return;
    }

    int unload_status = FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
    if (unload_status == 0) {
        try {
            std::cerr << std::format(
                "Unload library {} faild - just saying\n", m_library_path
            );
        } catch (...) {
            // Just log, nothing bad, can ignore
        }
    }
}
