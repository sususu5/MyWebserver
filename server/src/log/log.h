#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <mutex>
#include <string>
#include <thread>
#include "../buffer/buffer.h"
#include "blockdeque.h"

class Log {
public:
    // Initialize the log (path, suffix, maxQueueCapacity)
    void init(int level, const char* path = "./log", const char* suffix = ".log", int maxQueCapacity = 1024);

    static Log* Instance();
    // The public method to write log asynchronously, calls the private method AsyncWrite_
    static void FlushLogThread();

    // Format the output content according to the standard format
    void write(int level, const char* format, ...);
    // Write the log content in the buffer to the log file immediately
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; }

private:
    Log();
    virtual ~Log();
    void AppendLogLevelTitle_(int level);
    // Write the log content in the deque into the log file
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;  // The maximum length of the log path
    static const int LOG_NAME_LEN = 256;  // The maximum length of the log name
    static const int MAX_LINES = 50000;   // The maximum number of lines in a log file

    const char* path_;
    const char* suffix_;

    int MAX_LINES_;

    int lineCount_;
    int toDay_;

    bool isOpen_;

    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE* fp_;                                        // The file pointer to the log file
    std::unique_ptr<BlockDeque<std::string>> deque_;  // The queue to store the log content
    std::unique_ptr<std::thread> writeThread_;  // The thread to write the log content from the queue to the log file
    std::mutex mtx_;                            // The mutex to protect the log content
};

#define LOG_BASE(level, format, ...)                     \
    do {                                                 \
        Log* log = Log::Instance();                      \
        if (log->IsOpen() && log->GetLevel() <= level) { \
            log->write(level, format, ##__VA_ARGS__);    \
            log->flush();                                \
        }                                                \
    } while (0);

// Four levels of log, used to output different levels of log information
// ... indicates variadic arguments, and __VA_ARGS__ represents the values of ... being copied here.
// Adding ## in front ensures that if the number of variadic arguments is 0, the extra "," will be removed,
// preventing compilation errors.
#define LOG_DEBUG(format, ...)             \
    do {                                   \
        LOG_BASE(0, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_INFO(format, ...)              \
    do {                                   \
        LOG_BASE(1, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_WARN(format, ...)              \
    do {                                   \
        LOG_BASE(2, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_ERROR(format, ...)             \
    do {                                   \
        LOG_BASE(3, format, ##__VA_ARGS__) \
    } while (0);

#endif  // LOG_H