#include "buffer.h"

Buffer::Buffer(int initBuffSize): buffer_(initBuffSize), readPos_(0), writePos_(0) {}


size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::PrependableBytes() const {
    return readPos_;
}


const char* Buffer::Peek() const {
    return &buffer_[readPos_];
}

void Buffer::EnsureWritable(size_t len) {
    if (len > WritableBytes()) {
        MakeSpace_(len);
    }
    assert(len <= WritableBytes());
}

void Buffer::HasWritten(size_t len) {
    writePos_ += len;
}


void Buffer::Retrieve(size_t len) {
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end);
    Retrieve(end - Peek());
}


// Set readPos_ and writePos_ to 0 and clear the buffer
void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}


const char* Buffer::BeginWriteConst() const {
    return &buffer_[writePos_];
}

char* Buffer::BeginWrite() {
    return &buffer_[writePos_];
}


// Append data to the end of the buffer
void Buffer::Append(const char* str, size_t len) {
    assert(str);                                // Check the str is not nullptr
    EnsureWritable(len);                        // Ensure the buffer has enough space to store len bytes
    std::copy(str, str + len, BeginWrite());    // Copy len bytes from str to the buffer
    HasWritten(len);                            // Update the writePos_
}

void Buffer::Append(const std::string &str) {
    // c_str() returns a pointer to an array that contains a null-terminated sequence of characters
    Append(str.c_str(), str.size());
}

void Buffer::Append(const void* data, size_t len) {
    // Type casting
    Append(static_cast<const char*>(data), len);
}

// Append another buffer to the end of the buffer
void Buffer::Append(const Buffer &buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}


ssize_t Buffer::ReadFd(int fd, int* Errno) {
    char buff[65535];
    struct iovec iov[2];
    size_t writable = WritableBytes();
    
    // Ensure the buffer has enough space to store data
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *Errno = errno;
    } else if (static_cast<size_t>(len) <= writable) {
        writePos_ += len;
    } else {
        writePos_ = buffer_.size();
        Append(buff, static_cast<size_t>(len - writable));
    }
    return len;
}

ssize_t Buffer::WriteFde(int fd, int* Errno) {
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0) {
        *Errno = errno;
        return len;
    }
    Retrieve(len);
    return len;
}


char* Buffer::BeginPtr_() {
    return &buffer_[0];
}

const char* Buffer::BeginPtr_() const {
    return &buffer_[0];
}

// Expand the buffer when it is full
void Buffer::MakeSpace_(size_t len) {
    // The current left space is not enough
    if (WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } else {
        // The current left space is enough, move the readable bytes to the front
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == ReadableBytes());
    }
}