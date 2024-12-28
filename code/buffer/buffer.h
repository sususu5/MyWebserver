#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer {
public:
    // The default size of buffer is 1024
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    // Return the numebr of writable, readable, prependable bytes
    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    // Return a pointer to the begin of the readable bytes
    const char* Peek() const;
    void EnsureWritable(size_t len);
    // Update the writePos_ after writing len bytes
    void HasWritten(size_t len);

    // Retrieve bytes with len from the buffer
    void Retrieve(size_t len);
    // Retrieve bytes until the end of the buffer
    void RetrieveUntil(const char* end);

    void RetrieveAll();
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const char* str, size_t len);
    void Append(const std::string &str);
    void Append(const void* data, size_t len);
    void Append(const Buffer &buff);

    // Read data from fd to buffer, return the number of bytes read
    ssize_t ReadFd(int fd, int* Errno);
    // Write data from buffer to fd, return the number of bytes written
    ssize_t WriteFde(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    // The index of the first readable byte, use atomic to ensure thread safety
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};

#endif