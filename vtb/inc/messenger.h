#pragma once

#include "logger.h"
#include <sstream>
#include <string>

namespace vtb {

    class LogStream {
    public:
        LogStream(LogLevel level, const std::string& prefix);
        ~LogStream();

        // Support for all standard types
        template<typename T>
        LogStream& operator<<(const T& value) {
            m_oss << value;
            return *this;
        }

    private:
        LogLevel m_level;
        std::string m_prefix;
        std::ostringstream m_oss;
    };

    // Global access functions
    LogStream info();
    LogStream error();
    LogStream details();
}
