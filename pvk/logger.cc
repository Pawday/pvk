#include <cstddef>
#include <format>
#include <string>

#include <pvk/log.hh>
#include <pvk/logger.hh>
#include <string_view>

#include "pvk/log_utils.hh"

namespace pvk {

struct Logger::Detail
{
    static void dispatch_line(
        const void *user_data, Logger::Level level, std::string_view &message
    ) noexcept
    try {
        const Logger *logger = reinterpret_cast<const Logger *>(user_data);

        std::string mangled_message;

        const std::string addr_name = std::format(
            "pvk::Logger at 0x{:x}", reinterpret_cast<size_t>(logger)
        );

        if (logger->m_source_name.empty()) {
            mangled_message = std::format("[{}] {}", addr_name, message);
        } else {
            mangled_message =
                std::format("[{}] {}", logger->m_source_name, message);
        }

        if (logger->m_callback != nullptr) {
            logger->m_callback(logger->m_user_data, level, mangled_message);
            return;
        }

        switch (level) {
        case Level::ERROR:
            pvk::error(mangled_message);
            break;
        case Level::WARNING:
            pvk::warning(mangled_message);
            break;
        case Level::INFO:
            pvk::info(mangled_message);
            break;
        case Level::DEBUG:
            pvk::debug(mangled_message);
            break;
        case Level::TRACE:
            pvk::trace(mangled_message);
            break;
        }

    } catch (...) {
    }

    static void dispatch(
        const void *user_data, Logger::Level level, const std::string &message
    ) noexcept
    try {
        auto lines = split_ln(message);

        for (auto line : lines) {
            std::string_view line_view(message.data() + line.start, line.size);
            dispatch_line(user_data, level, line_view);
        }

    } catch (...) {
    }
};

void Logger::error(const std::string &message) const noexcept
{
    Detail::dispatch(this, Logger::Level::ERROR, message);
}

void Logger::warning(const std::string &message) const noexcept
{
    Detail::dispatch(this, Logger::Level::WARNING, message);
}

void Logger::info(const std::string &message) const noexcept
{
    Detail::dispatch(this, Logger::Level::INFO, message);
}

void Logger::debug(const std::string &message) const noexcept
{
    Detail::dispatch(this, Logger::Level::DEBUG, message);
}

void Logger::trace(const std::string &message) const noexcept
{
    Detail::dispatch(this, Logger::Level::TRACE, message);
}

void Logger::set_name(const std::string &source_name) noexcept
try {
    m_source_name = source_name;
} catch (...) {
}

void Logger::set_callback(Callback new_callback) noexcept
{
    this->m_callback = new_callback;
}

} // namespace pvk
