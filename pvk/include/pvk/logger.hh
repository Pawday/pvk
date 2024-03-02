#pragma once

#include <optional>
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

    Logger() noexcept = default;
    Logger(Logger &&) = default;
    ~Logger() noexcept = default;

    Logger(const Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;

    using Callback = void (*)(void *, Level, const std::string &) noexcept;

    void error(const std::string &message) const noexcept;
    void warning(const std::string &message) const noexcept;
    void info(const std::string &message) const noexcept;
    void debug(const std::string &message) const noexcept;
    void trace(const std::string &message) const noexcept;

    void set_name(const std::string &source_name) noexcept;
    void set_callback(Callback new_callback) noexcept;
    void set_userdata(void *user_data) noexcept
    {
        m_user_data = user_data;
    }

  private:
    struct Detail;
    Callback m_callback = nullptr;
    void *m_user_data = nullptr;
    std::string m_source_name;
};

} // namespace pvk
