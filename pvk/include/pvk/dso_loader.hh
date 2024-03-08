#pragma once

#include <optional>
#include <string>
#include <utility>

namespace pvk {
struct SymLoader
{
    static std::optional<SymLoader> load(const std::string &library_file);
    ~SymLoader() noexcept;

    SymLoader(const SymLoader &) = delete;
    SymLoader &operator=(const SymLoader &) = delete;

    SymLoader(SymLoader &&other) noexcept;
    SymLoader &operator=(SymLoader &&other) noexcept
    {
        std::swap(*this, other);
        return *this;
    }

    std::optional<void *> load_sym(const std::string &symname);

  private:
    SymLoader();
    std::string m_library_path;
    void *m_handle = nullptr;
};
} // namespace pvk
