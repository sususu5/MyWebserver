#include "log.h"
#include <string>

Log::~Log() {
    if (is_async_ && deque_) {
        deque_->close();
        if (write_thread_ && write_thread_->joinable()) {
            while (!deque_->empty()) {
                deque_->flush();
            };
            write_thread_->join();
        }
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

int Log::get_level() {
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

void Log::set_level(int level) {
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}

void Log::init(int level = 1, const char* path, const char* suffix, int maxQueueSize) {
    is_open_ = true;
    level_ = level;
    if (maxQueueSize > 0) {
        is_async_ = true;
        if (!deque_) {
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            deque_ = std::move(newDeque);
            std::unique_ptr<std::thread> NewThread(new std::thread(flush_log_thread));
            write_thread_ = std::move(NewThread);
        }
    } else {
        is_async_ = false;
    }

    line_count_ = 0;
    path_ = path;
    suffix_ = suffix;

    // Create log directory if it doesn't exist
    struct stat st;
    if (stat(path_, &st) != 0) {
        mkdir(path_, 0777);
    }

    auto now = std::chrono::system_clock::now();
    auto now_days = std::chrono::floor<std::chrono::days>(now);
    auto ymd = std::chrono::year_month_day{now_days};

    std::string tail = std::format("{:%Y_%m_%d}", ymd);
    std::string file_name = std::format("{}/{}{}", path_, tail, suffix_);

    {
        std::lock_guard<std::mutex> locker(mtx_);
        // Clear the buffer
        buff_.retrieve_all();
        // If the file pointer is not null, close the file
        if (fp_) {
            flush();
            fclose(fp_);
        }

        fp_ = fopen(file_name.c_str(), "a");

        // Retry if failed (though directory check above should handle it)
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(file_name.c_str(), "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::flush() {
    if (is_async_) {
        deque_->flush();
    }
    fflush(fp_);
}

void Log::async_write_() {
    std::string str = "";
    while (deque_->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

Log* Log::instance() {
    static Log inst;
    return &inst;
}

void Log::flush_log_thread() { Log::instance()->async_write_(); }