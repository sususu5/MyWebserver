#pragma once

#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string>
#include <vector>

class Buffer {
    static constexpr size_t INIT_BUFFER_SIZE = 1024;

public:
    Buffer(int init_buffer_size = INIT_BUFFER_SIZE) : buffer_(init_buffer_size), readPos_(0), writePos_(0) {}
    ~Buffer() = default;

    size_t readable_bytes() const { return writePos_ - readPos_; };
    size_t writable_bytes() const { return buffer_.size() - writePos_; }
    // The number of bytes that can be written to the buffer
    size_t prependable_bytes() const { return readPos_; };

    // Return a pointer to the begin of the readable bytes
    const char* peek() const { return begin_ptr() + readPos_; };
    // If the space is not enough, resize the buffer
    void ensure_writable(size_t len);
    // This function updates the writePos_ after writing len bytes
    void has_written(size_t len) { writePos_ += len; };

    // These functions are used to read data from the buffer
    void retrieve(size_t len);
    void retrieve_until(const char* end);
    void retrieve_all();
    std::string retrieve_all_to_str();

    char* begin_write() { return begin_ptr() + writePos_; };
    const char* begin_write_const() const { return begin_ptr() + writePos_; };

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);

    // Read / write data from a file descriptor to the buffer
    ssize_t read_fd(int fd, int* Errno);
    ssize_t write_fd(int fd, int* Errno);
    struct iovec ToIovec();

private:
    char* begin_ptr() { return &*buffer_.begin(); };
    const char* begin_ptr() const { return &*buffer_.begin(); };
    void make_space(size_t len);

    std::vector<char> buffer_;
    // The index of the first readable and writable byte, use atomic to ensure thread safety
    std::size_t readPos_;
    std::size_t writePos_;
};
