#include "buffer.h"
#include <cstring>

void Buffer::ensure_writable(size_t len) {
    if (writable_bytes() < len) {
        make_space(len);
    }
    assert(writable_bytes() >= len);
}

void Buffer::retrieve(size_t len) {
    assert(len <= readable_bytes());
    readPos_ += len;
}

void Buffer::retrieve_until(const char* end) {
    assert(peek() <= end);
    retrieve(end - peek());
}

void Buffer::retrieve_all() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::retrieve_all_to_str() {
    std::string str(peek(), readable_bytes());
    retrieve_all();
    return str;
}

// .data() returns a pointer to the first element in the array
//  used internally by the string to store its owned elements.
void Buffer::append(const std::string& str) { append(str.data(), str.length()); }

void Buffer::append(const char* str, size_t len) {
    assert(str);
    ensure_writable(len);
    std::copy(str, str + len, begin_write());
    has_written(len);
}

void Buffer::append(const void* data, size_t len) {
    assert(data);
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const Buffer& buff) { append(buff.peek(), buff.readable_bytes()); }

ssize_t Buffer::read_fd(int fd, int* saveErrno) {
    // The buffer is used to store the data that cannot be read into the buffer
    char buff[65535];
    // The iovec structure is used to gather scattered I/O
    // It is often used with readv() and writev() to read or write multiple buffers in a single I/O operation
    struct iovec iov[2];
    const size_t writable = writable_bytes();
    // Ensure all the data can be read into the buffer by using two iovec
    iov[0].iov_base = begin_ptr() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(len) <= writable) {
        writePos_ += len;
    } else {
        writePos_ = buffer_.size();
        append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::write_fd(int fd, int* saveErrno) {
    // This function writes the data in the buffer to the file descriptor
    // so needs to get the size of the readable bytes
    auto readSize = readable_bytes();
    auto len = write(fd, peek(), readSize);
    if (len < 0) {
        *saveErrno = errno;
        return len;
    }
    readPos_ += len;
    return len;
}

void Buffer::make_space(size_t len) {
    if (writable_bytes() + prependable_bytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = readable_bytes();
        std::copy(begin_ptr() + readPos_, begin_ptr() + writePos_, begin_ptr());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == readable_bytes());
    }
}
