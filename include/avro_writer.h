#pragma once

#include <sstream>
#include "binary_writer.h"
#include "avro_header.h"
#include "block_stream.h"

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
    void set_meta_internal(std::string key, std::string value)
    {
        header_.meta.insert(std::make_pair(std::move(key), std::vector<char>(value.begin(), value.end())));
    }

    void set_meta_internal(std::string key, std::vector<char> value)
    {
        header_.meta.insert(std::make_pair(std::move(key), std::move(value)));
    }

public:
    avro_writer(std::ostream &stream, std::string schema) noexcept : output_writer_{stream}, block_stream_{},
                                                                     block_writer_{block_stream_},
                                                                     header_{}, record_count_{}, is_header_written_{}
    {
        header_.init_magic();
        header_.generate_random_sync();
        set_meta_internal("avro.codec", "null");
        set_meta_internal("avro.schema", std::move(schema));
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
            const char *ptr = block_stream_.ptr();
            output_writer_.write(record_count_);
            output_writer_.write(size);
            output_writer_.write(ptr, size);
            output_writer_.write(header_.sync);

            record_count_ = 0;
            block_stream_.seekp(0);
        }
    }
};
} // namespace avro
