#pragma once

#include <cstdint>
#include <limits>

namespace avro::internal
{
template <size_t Size>
constexpr static bool is_valid_positive_size_v = Size > 0 && Size <= static_cast<size_t>(std::numeric_limits<int64_t>::max());

template <std::size_t I, class T>
struct indexed 
{
    using type = T;
};

template <class Is, class ...Ts>
struct indexer;

template <std::size_t ...Is, typename ...Ts>
struct indexer<std::index_sequence<Is...>, Ts...> : indexed<Is, Ts>...
{
};

template <std::size_t I, typename T>
static indexed<I, T> select(indexed<I, T>);

template <std::size_t I, typename ...Ts>
using nth_element_t = typename decltype(select<I>(indexer<std::index_sequence_for<Ts...>, Ts...>{}))::type;
}
