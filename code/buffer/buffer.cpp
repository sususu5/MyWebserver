#include "buffer.h"

Buffer::Buffer(int initBuffSize): buffer_(initBuffSize), readPos_(0), writePos_(0) {}

size_t Buffer::readableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::writableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::prependableBytes() const {
    return readPos_;
}

const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

void Buffer::EnsureWritable(size_t len) {
    if(writableBytes() < len) {
        MakeSpace_(len);
    }
    assert(writableBytes() >= len);
}

void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Retrieve(size_t len) {
    assert(len <= readableBytes());
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), readableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

// .data() returns a pointer to the first element in the array
//  used internally by the string to store its owned elements.
void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.readableBytes());
}

ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    // The buffer is used to store the data that cannot be read into the buffer
    char buff[65535];
    // The iovec structure is used to gather scattered I/O
    // It is often used with readv() and writev() to read or write multiple buffers in a single I/O operation
    struct iovec iov[2];
    const size_t writable = writableBytes();
    // Ensure all the data can be read into the buffer by using two iovec
    iov[0].iov_base = BeginPtr_() + writePos_;
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
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    // This function writes the data in the buffer to the file descriptor
    // so needs to get the size of the readable bytes
    size_t readSize = readableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if (len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

// This function changes an iterator to a pointer
const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    if (writableBytes() + prependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        size_t readable = readableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == readableBytes());
    }
}