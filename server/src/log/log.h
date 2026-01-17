#pragma once

#include <assert.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <chrono>
#include <format>
#include <mutex>
#include <string>
#include <thread>
#include "../buffer/buffer.h"
#include "blockdeque.h"

class Log {
public:
    void init(int level, const char* path = "./log", const char* suffix = ".log", int maxQueCapacity = 1024);

    static Log* instance();
    // The public method to write log asynchronously, calls the private method AsyncWrite_
    static void flush_log_thread();

    template <typename... Args>
    void write(int level, std::format_string<Args...> fmt, Args&&... args) {
        auto now = std::chrono::system_clock::now();
        auto now_days = std::chrono::floor<std::chrono::days>(now);
        auto ymd = std::chrono::year_month_day{now_days};
        int current_day = static_cast<unsigned>(ymd.day());

        // If the day changes or the number of lines reaches the maximum, create a new log file
        if (today_ != current_day || (line_count_ && (line_count_ % MAX_LINES == 0))) {
            std::unique_lock<std::mutex> locker(mtx_);
            locker.unlock();

            // The following process is thread safe, so we can unlock the mutex,
            // but the conditional statement needs to be locked
            std::string tail = std::format("{:%Y_%m_%d}", ymd);
            std::string new_file;

            if (today_ != current_day) {
                new_file = std::format("{}/{}{}", path_, tail, suffix_);
                today_ = current_day;
                line_count_ = 0;
            } else {
                new_file = std::format("{}/{}-{}{}", path_, tail, (line_count_ / MAX_LINES), suffix_);
            }

            locker.lock();
            flush();
            if (fp_) fclose(fp_);
            fp_ = fopen(new_file.c_str(), "a");
            assert(fp_ != nullptr);
        }

        {
            std::unique_lock<std::mutex> locker(mtx_);
            line_count_++;
            auto out = buff_.begin_write();
            auto now_us =
                std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;
            out = std::format_to(out, "{:%Y-%m-%d %H:%M:%S}.{:06d} {} ", std::chrono::floor<std::chrono::seconds>(now),
                                 now_us, get_level_string_(level));
            out = std::format_to(out, fmt, std::forward<Args>(args)...);
            *out++ = '\n';
            *out++ = '\0';

            buff_.has_written(out - buff_.begin_write());

            if (is_async_ && deque_ && !deque_->full()) {
                deque_->push_back(buff_.retrieve_all_to_str());
            } else {
                fputs(buff_.peek(), fp_);
            }
            buff_.retrieve_all();
        }
    }

    // Write the log content in the buffer to the log file immediately
    void flush();

    int get_level();
    void set_level(int level);
    bool is_open() { return is_open_; }

private:
    static constexpr std::string_view get_level_string_(int level) {
        switch (level) {
            case 0:
                return "[debug]:";
            case 1:
                return "[info] :";
            case 2:
                return "[warn] :";
            case 3:
                return "[error]:";
            default:
                return "[info]: ";
        }
    }

    Log() : line_count_(0), is_async_(false), write_thread_(nullptr), deque_(nullptr), today_(0), fp_(nullptr){};
    virtual ~Log();
    // Write the log content in the deque into the log file
    void async_write_();

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

    int MAX_LINES_;

    int line_count_;
    int today_;

    bool is_open_;

    Buffer buff_;
    int level_;
    bool is_async_;

    FILE* fp_;                                        // The file pointer to the log file
    std::unique_ptr<BlockDeque<std::string>> deque_;  // The queue to store the log content
    std::unique_ptr<std::thread> write_thread_;  // The thread to write the log content from the queue to the log file
    std::mutex mtx_;                             // The mutex to protect the log content
};

#define LOG_BASE(level, format, ...)                       \
    do {                                                   \
        Log* log = Log::instance();                        \
        if (log->is_open() && log->get_level() <= level) { \
            log->write(level, format, ##__VA_ARGS__);      \
            log->flush();                                  \
        }                                                  \
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
