#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace avro::internal
{
template <size_t Size>
constexpr static bool is_valid_positive_size_v = Size > 0 && Size <= static_cast<size_t>(std::numeric_limits<int64_t>::max());

template <std::size_t I, class T>
struct indexed
{
    using type = T;
};

template <class Is, class... Ts>
struct indexer;

template <std::size_t... Is, typename... Ts>
struct indexer<std::index_sequence<Is...>, Ts...> : indexed<Is, Ts>...
{
};

template <std::size_t I, typename T>
static indexed<I, T> select(indexed<I, T>);

template <std::size_t I, typename... Ts>
using nth_element_t = typename decltype(select<I>(indexer<std::index_sequence_for<Ts...>, Ts...>{}))::type;

class blockstreambuf : public std::stringbuf
{
public:
    inline char *eback()
    {
        return std::streambuf::eback();
    }

    inline char *pptr()
    {
        return std::streambuf::pptr();
    }
};

class blockstream : public std::ostringstream
{
    blockstreambuf buffer_;

public:
    blockstream()
    {
        std::basic_ostream<char>::rdbuf(&buffer_);
    }

    inline char *ptr()
    {
        //assert(tellp() == (d_buf.pptr() - d_buf.eback()));
        return buffer_.eback();
    }
};
} // namespace avro::internal

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

    template <class T>
    inline void write(const std::optional<T> &value) const // union [null, T]
    {
        if (value.has_value())
        {
            write_signed_integer(1);
            write(value.value());
        }
        else
        {
            write_zero();
        }
    }

    template <class... Args>
    inline void write(const std::variant<Args...> &value) const // union
    {
        write(value.index());
        std::visit([*this](auto &&item) { write(item); }, value);
    }

    template <class T, class = decltype(std::declval<T &>().write_to(std::declval<const binary_writer>()))>
    inline void write(const T &value) const // record
    {
        value.write_to(*this);
    }

    template <class... Args>
    inline void write(const std::unique_ptr<Args...> &ptr) const // union [null, T]
    {
        if (ptr)
        {
            write_signed_integer(1);
            write(*ptr);
        }
        else
        {
            write_zero();
        }
    }

    template <class... Args>
    inline void write(const std::shared_ptr<Args...> &ptr) const // union [null, T]
    {
        if (ptr)
        {
            write_signed_integer(1);
            write(*ptr);
        }
        else
        {
            write_zero();
        }
    }
};
} // namespace avro

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

namespace avro
{
template <class T>
class avro_writer
{
    binary_writer output_writer_;
    internal::blockstream block_stream_;
    binary_writer block_writer_;
    internal::header header_;
    size_t record_count_;
    bool is_header_written_;

private:
    inline static const std::string avro_prefix_{"avro."};

    inline void set_meta_internal(std::string key, std::vector<char> value)
    {
        header_.meta.insert(std::make_pair(std::move(key), std::move(value)));
    }

    inline void set_meta_internal(std::string key, std::string value)
    {
        set_meta_internal(std::move(key), std::vector<char>(value.begin(), value.end()));
    }

    inline void check_meta_key(const std::string &key)
    {
        if (key.length() >= avro_prefix_.length() &&
            std::equal(avro_prefix_.begin(), avro_prefix_.end(), key.begin()))
        {
            throw std::invalid_argument{"The key names 'avro.' are preserved for internal use."};
        }
    }

public:
    avro_writer(std::ostream &stream, std::string schema)
        : output_writer_{stream}, block_stream_{},
          block_writer_{block_stream_},
          header_{}, record_count_{}, is_header_written_{}
    {
        header_.init_magic();
        header_.generate_random_sync();
        set_meta_internal("avro.codec", "null");
        set_meta_internal("avro.schema", std::move(schema));
    }

    ~avro_writer()
    {
        flush();
    }

    void set_meta(std::string key, std::string value)
    {
        check_meta_key(key);
        set_meta_internal(std::move(key), std::move(value));
    }

    void set_meta(std::string key, std::vector<char> value)
    {
        check_meta_key(key);
        set_meta_internal(std::move(key), std::move(value));
    }

    void write(const T &record)
    {
        ++record_count_;
        block_writer_.write(record);
    }

    void flush()
    {
        if (!is_header_written_)
        {
            output_writer_.write(header_);
            is_header_written_ = true;
        }

        if (record_count_)
        {
            size_t size = block_stream_.tellp();
            output_writer_.write(record_count_);
            output_writer_.write(size);
            output_writer_.write(block_stream_.ptr(), size);
            output_writer_.write(header_.sync);

            record_count_ = 0;
            block_stream_.seekp(0);
        }
    }
};
} // namespace avro
