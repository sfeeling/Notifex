//
// Created by sfeeling on 19-4-25.
//

#ifndef NOTIFEX_BUFFER_H
#define NOTIFEX_BUFFER_H

#include <algorithm>
#include <vector>

#include <cassert>
#include <cstring>

#include "Endian.h"
#include "StringPiece.h"
#include "Types.h"

namespace notifex
{

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + initialSize),
              readerIndex_(kCheapPrepend),
              writerIndex_(kCheapPrepend)
    {
        assert(ReadableBytes() == 0);
        assert(WritableBytes() == initialSize);
        assert(PrependableBytes() == kCheapPrepend);
    }

    size_t ReadableBytes() const
    { return writerIndex_ - readerIndex_; }

    size_t WritableBytes() const
    { return buffer_.size() - writerIndex_; }

    size_t PrependableBytes() const
    { return readerIndex_; }

    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    const char* peek() const
    { return begin() + readerIndex_; }

    const char* findCRLF() const
    {
        // FIXME: replace with memmem()?
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        // FIXME: replace with memmem()?
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', ReadableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    void retrieve(size_t len)
    {
        assert(len <= ReadableBytes());
        if (len < ReadableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveInt64()
    {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    string retrieveAllAsString()
    {
        return retrieveAsString(ReadableBytes());
    }

    string retrieveAsString(size_t len)
    {
        assert(len <= ReadableBytes());
        string result(peek(), len);
        retrieve(len);
        return result;
    }

    StringPiece toStringPiece() const
    {
        return StringPiece(peek(), static_cast<int>(ReadableBytes()));
    }

    void append(const StringPiece& str)
    {
        append(str.data(), str.size());
    }

    void append(const char* /*restrict*/ data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const void* /*restrict*/ data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void ensureWritableBytes(size_t len)
    {
        if (WritableBytes() < len)
        {
            makeSpace(len);
        }
        assert(WritableBytes() >= len);
    }

    char* beginWrite()
    { return begin() + writerIndex_; }

    const char* beginWrite() const
    { return begin() + writerIndex_; }

    void hasWritten(size_t len)
    {
        assert(len <= WritableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len)
    {
        assert(len <= ReadableBytes());
        writerIndex_ -= len;
    }

    ///
    /// Append int64_t using network endian
    ///
    void appendInt64(int64_t x)
    {
        int64_t be64 = HostToNetwork64(x);
        append(&be64, sizeof be64);
    }

    ///
    /// Append int32_t using network endian
    ///
    void appendInt32(int32_t x)
    {
        int32_t be32 = HostToNetwork32(x);
        append(&be32, sizeof be32);
    }

    void appendInt16(int16_t x)
    {
        int16_t be16 = HostToNetwork16(x);
        append(&be16, sizeof be16);
    }

    void appendInt8(int8_t x)
    {
        append(&x, sizeof x);
    }

    ///
    /// Read int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int64_t readInt64()
    {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    ///
    /// Read int32_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int32_t readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    ///
    /// Peek int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int64_t)
    int64_t peekInt64() const
    {
        assert(ReadableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        return NetworkToHost64(be64);
    }

    ///
    /// Peek int32_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int32_t peekInt32() const
    {
        assert(ReadableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return NetworkToHost32(be32);
    }

    int16_t peekInt16() const
    {
        assert(ReadableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return NetworkToHost16(be16);
    }

    int8_t peekInt8() const
    {
        assert(ReadableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    ///
    /// Prepend int64_t using network endian
    ///
    void prependInt64(int64_t x)
    {
        int64_t be64 = HostToNetwork64(x);
        prepend(&be64, sizeof be64);
    }

    ///
    /// Prepend int32_t using network endian
    ///
    void prependInt32(int32_t x)
    {
        int32_t be32 = HostToNetwork32(x);
        prepend(&be32, sizeof be32);
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = HostToNetwork16(x);
        prepend(&be16, sizeof be16);
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof x);
    }

    void prepend(const void* /*restrict*/ data, size_t len)
    {
        assert(len <= PrependableBytes());
        readerIndex_ -= len;
        auto d = static_cast<const char*>(data);
        std::copy(d, d+len, begin()+readerIndex_);
    }

    void shrink(size_t reserve)
    {
        // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
        Buffer other;
        other.ensureWritableBytes(ReadableBytes()+reserve);
        other.append(toStringPiece());
        swap(other);
    }

    size_t internalCapacity() const
    {
        return buffer_.capacity();
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t ReadFd(int fd, int *savedErrno);

private:

    char* begin()
    { return &*buffer_.begin(); }

    const char* begin() const
    { return &*buffer_.begin(); }

    void makeSpace(size_t len)
    {
        if (WritableBytes() + PrependableBytes() < len + kCheapPrepend)
        {
            // FIXME: move readable data
            buffer_.resize(writerIndex_+len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = ReadableBytes();
            std::copy(begin()+readerIndex_,
                      begin()+writerIndex_,
                      begin()+kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == ReadableBytes());
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];

};

}   // namespace notifex




#endif //NOTIFEX_BUFFER_H
