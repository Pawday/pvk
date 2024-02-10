#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

struct SymLoader
{
    static std::optional<SymLoader> load(const std::string &library_file);
    ~SymLoader() noexcept;

    SymLoader(const SymLoader &) = delete;
    SymLoader &operator=(const SymLoader &) = delete;

    friend void swap(SymLoader &lhs, SymLoader rhs) noexcept
    {
        std::swap(lhs.m_moved, rhs.m_moved);
        std::swap(lhs.m_library_path, rhs.m_library_path);
        std::swap(lhs.m_handle, rhs.m_handle);
    }

    SymLoader(SymLoader &&other) noexcept;
    SymLoader &operator=(SymLoader &&other) noexcept
    {
        std::swap(*this, other);
        return *this;
    }

    std::optional<void *> load_sym(const std::string &symname);

  private:
    SymLoader();
    bool m_moved = false;
    std::string m_library_path;
    void *m_handle = NULL;
};
