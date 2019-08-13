#pragma once

#include <sstream>

namespace avro::internal
{
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
