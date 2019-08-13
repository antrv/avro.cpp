#pragma once

#include <algorithm>
#include <random>
#include <unordered_map>
#include "fixed.h"
#include "binary_writer.h"

namespace avro::internal
{
class header
{
public:
    using magic_t = fixed<4>;
    using meta_t = std::unordered_map<std::string, std::vector<char>>;
    using sync_t = fixed<16>;

    magic_t magic;
    meta_t meta;
    sync_t sync;

    void write_to(const binary_writer writer) const
    {
        writer.write(magic);
        writer.write(meta);
        writer.write(sync);
    }

    void init_magic()
    {
        magic = magic_t::array_t{'O', 'b', 'j', 1};
    }

    void generate_random_sync()
    {
        std::random_device random_device{};
        std::mt19937 generator{random_device()};
        const std::uniform_int_distribution<int32_t> distribution{
            std::numeric_limits<char>::min(),
            std::numeric_limits<char>::max()};

        sync_t::array_t array;
        std::generate(array.begin(), array.end(), [&]() { return distribution(generator); });
        sync = array;
    }
};
} // namespace avro::internal
