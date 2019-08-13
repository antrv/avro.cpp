#pragma once

#include <cstdint>
#include <limits>
#include <ostream>
#include "utilities.h"

namespace avro
{
class binary_writer;

template <size_t Size, class = std::enable_if_t<internal::is_valid_positive_size_v<Size>>>
class fixed
{
public:
    using array_t = std::array<char, Size>;

private:
    array_t array_;

public:
    constexpr fixed() noexcept = default;

    constexpr fixed(const array_t &arr) noexcept : array_{arr}
    {
    }

    constexpr fixed(const fixed &f) noexcept : array_{f.array_}
    {
    }

    inline static constexpr size_t size() noexcept
    {
        return Size;
    }

    inline void write_to(binary_writer writer) const
    {
        writer.write(array_);
    }
};
} // namespace avro
