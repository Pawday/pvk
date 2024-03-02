#pragma once

#include <string>

namespace pvk {

struct Logger
{
    enum class Level
    {
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        TRACE
    };

    using Callback = void (*)(void *, Level, const std::string &) noexcept;

    void error(const std::string &message) noexcept;
    void warning(const std::string &message) noexcept;
    void info(const std::string &message) noexcept;
    void debug(const std::string &message) noexcept;
    void trace(const std::string &message) noexcept;

    void set_source(const std::string &source_name) noexcept;
    void set_callback(Callback new_callback);

  private:
    Callback m_callback = nullptr;
};

} // namespace pvk