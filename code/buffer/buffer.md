# Buffer

### This buffer is maintained by moving the reading index and writing index in a vector.
### The ReadFd is called by HttpConn to read data to the buffer.
### The WriteFd is responsible for writing readable data in the buffer to file descriptors.