#pragma once

#include <cstdint>
#include <limits>

namespace avro::internal
{
template <size_t Size>
constexpr static bool is_valid_positive_size_v = Size > 0 && Size <= static_cast<size_t>(std::numeric_limits<int64_t>::max());
}
