#pragma once

#include <stdexcept>
#include <string>

namespace strings
{
    template <typename... Args>
    inline std::string format(const std::string_view& format, Args... args)
    {
        int size = std::snprintf(nullptr, 0, format.data(), args...) + 1;
        if (size <= 0)
        {
            throw std::runtime_error("Error during formatting");
        }
        std::string result;
        result.resize(size);
        std::snprintf(result.data(), size, format.data(), args...);
        return result;
    }
};
