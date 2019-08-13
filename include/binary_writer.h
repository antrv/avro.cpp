#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "fixed.h"

namespace avro
{
class binary_writer
{
    std::ostream &stream_;

    template <class T, class = std::enable_if_t<std::is_signed_v<T>>>
    inline void write_signed_integer(T value) const
    {
        using unsigned_t = std::make_unsigned_t<T>;
        auto n = static_cast<unsigned_t>((value << 1) ^ (value >> (8 * sizeof(T) - 1)));
        while ((n & ~static_cast<unsigned_t>(0x7F)) != 0)
        {
            stream_.put((n & 0x7F) | 0x80);
            n >>= 7;
        }

        stream_.put(n);
    }

public:
    binary_writer(std::ostream &stream) noexcept : stream_{stream}
    {
    }

    inline void write_zero() const
    {
        stream_.put(0);
    }

    inline void write(nullptr_t value) const
    {
    }

    inline void write(bool value) const
    {
        stream_.put(value ? 1 : 0);
    }

    inline void write(const int32_t value) const
    {
        write_signed_integer(value);
    }

    inline void write(const int64_t value) const
    {
        write_signed_integer(value);
    }

    inline void write(const size_t value) const
    {
        if constexpr (sizeof(size_t) < sizeof(int64_t))
        {
            write_signed_integer(static_cast<int64_t>(value));
            return;
        }

        if (value < static_cast<size_t>(std::numeric_limits<int64_t>::max()))
        {
            write_signed_integer(static_cast<int64_t>(value));
            return;
        }

        throw std::out_of_range{"value is too big"};
    }

    inline void write(const float value) const
    {
        stream_.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }

    inline void write(const double value) const
    {
        stream_.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }

    inline void write(const char *data, const size_t size) const
    {
        stream_.write(data, size);
    }

    template <size_t Size>
    inline void write(const fixed<Size> &value) const // fixed
    {
        value.write_to(stream_);
    }

    template <size_t Size>
    inline void write(const std::array<char, Size> &value) const // fixed
    {
        stream_.write(value.data(), Size);
    }

    template <class... Args>
    inline void write(const std::vector<char, Args...> &value) const // bytes
    {
        if (value.empty())
        {
            write_zero();
        }
        else
        {
            write(value.size());
            stream_.write(value.data(), value.size());
        }
    }

    inline void write(const std::string &value) const
    {
        if (value.empty())
        {
            write_zero();
        }
        else
        {
            write(value.size());
            stream_.write(value.data(), value.size());
        }
    }

    inline void write(const std::string_view &value) const
    {
        if (value.empty())
        {
            write_zero();
        }
        else
        {
            write(value.size());
            stream_.write(value.data(), value.size());
        }
    }

    template <class T, size_t Size, class = std::enable_if_t<internal::is_valid_positive_size_v<Size>>>
    inline void write(const std::array<T, Size> &value) const // array
    {
        if constexpr (Size != 0)
        {
            write_signed_integer(static_cast<int64_t>(Size));
            for (const auto &item : value)
            {
                write(item);
            }
        }

        write_zero();
    }

    template <class... Args>
    inline void write(const std::vector<Args...> &value) const // array
    {
        if (!value.empty())
        {
            write(value.size());
            for (const auto &item : value)
            {
                write(item);
            }
        }

        write_zero();
    }

    template <class T>
    inline void write(const std::pair<const std::string, T> &value) const // for map
    {
        write(value.first);
        write(value.second);
    }

    template <class... Args>
    inline void write(const std::map<std::string, Args...> &value) const // map
    {
        if (!value.empty())
        {
            write(value.size());
            for (const auto &item : value)
            {
                write(item);
            }
        }

        write_zero();
    }

    template <class... Args>
    inline void write(const std::unordered_map<std::string, Args...> &value) const // map
    {
        if (!value.empty())
        {
            write(value.size());
            for (const auto &item : value)
            {
                write(item);
            }
        }

        write_zero();
    }

    template <class T, class = decltype(std::declval<T &>().write_to(std::declval<const binary_writer>()))>
    inline void write(const T &value) const // record
    {
        value.write_to(*this);
    }
};
} // namespace avro
