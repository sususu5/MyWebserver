#ifndef BUFFER_H
#define BUFFER_H

#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <vector>

class Buffer {
public:
    // The default size of buffer is 1024
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t writableBytes() const;
    size_t readableBytes() const;
    // Return the number of prependable bytes (the bytes before the readable bytes)
    // These bytes has been read but not processes
    size_t prependableBytes() const;

    // Return a pointer to the begin of the readable bytes
    const char* Peek() const;
    // If the space is not enough, resize the buffer
    void EnsureWritable(size_t len);
    // This function updates the writePos_ after writing len bytes
    void HasWritten(size_t len);

    // These functions are used to read data from the buffer
    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    // Read / write data from a file descriptor to the buffer
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    // The index of the first readable and writable byte, use atomic to ensure thread safety
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};

#endif  // BUFFER_H