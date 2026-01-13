#pragma once

#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <vector>

class Buffer {
    static constexpr size_t INIT_BUFFER_SIZE = 1024;

public:
    Buffer(int initBuffSize = INIT_BUFFER_SIZE);
    ~Buffer() = default;

    size_t readable_bytes() const;
    size_t writable_bytes() const;
    // The number of bytes that can be written to the buffer
    size_t prependable_bytes() const;

    // Return a pointer to the begin of the readable bytes
    const char* peek() const;
    // If the space is not enough, resize the buffer
    void ensure_writable(size_t len);
    // This function updates the writePos_ after writing len bytes
    void has_written(size_t len);

    // These functions are used to read data from the buffer
    void retrieve(size_t len);
    void retrieve_until(const char* end);
    void retrieve_all();
    std::string retrieve_all_to_str();

    const char* begin_write_const() const;
    char* begin_write();

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);

    // Read / write data from a file descriptor to the buffer
    ssize_t read_fd(int fd, int* Errno);
    ssize_t write_fd(int fd, int* Errno);

private:
    char* begin_ptr();
    const char* begin_ptr() const;
    void make_space(size_t len);

    std::vector<char> buffer_;
    // The index of the first readable and writable byte, use atomic to ensure thread safety
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};
