#include "log.h"

Log::Log() {
    fp_= nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    lineCount_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}

Log::~Log() {
    // Wake up the writing thread and 
    while (!deque_->empty()) {
        deque_->flush();
    }
    // Close the deque
    deque_->close();
    // Wait for the writing thread to finish
    writeThread_->join();
    // Close the log file
    if (fp_) {
        lock_guard<mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

void Log::flush() {
    if (isAsync_) {
        deque_->flush();
    }
    // Clear the input buffer
    fflush(fp_);
}

// Lazy initialization
Log* Log::Instance() {
    static Log log;
    return &log;
}

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}

void Log::AsyncWrite_() {
    string str = "";
    while (deque_->pop(str)) {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

// Initialize the log (path, suffix, maxQueueCapacity)
void Log::init(int level, const char* path, const char* suffix, int maxQueCapacity) {
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    if (maxQueCapacity) {
        // Asynchronous mode
        isAsync_ = true;
        // If the queue is empty, create a new queue
        if (!deque_) {
            unique_ptr<BlockQueue<std::string>> newQue(new BlockQueue<std::string>);
            deque_ = move(newQue);
            unique_ptr<thread> newThread(new thread(FlushLogThread));
            writeThread_ = move(newThread);
        }
    } else {
        isAsync_ = false;
    }

    lineCount_ = 0;

    // Get the current time
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    // Generate the log file name
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        // Create the log file and open it
        lock_guard<mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_) {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::write(int level, const char* format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    // If the current day is different from the day when the log file was created,
    // or the number of lines in the log file reaches the maximum, create a new log file
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))) {
        unique_lock<mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    // Generate a new log content in the buffer
    {
        unique_lock<mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);

        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        // Add the log content to the deque, wait for the writing thread to write to the log file
        if (isAsync_ && deque_ && !deque_->full()) {
            deque_->push_back(buff_.RetrieveAllToStr());
        } else {
            // Write the log content to the log file directly
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}

void Log::AppendLogLevelTitle_(int level) {
    switch (level) {
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info]:", 9);
        break;
    case 2:
        buff_.Append("[warn]:", 9);
        break;
    case 3:
        buff_.Append("[error]:", 9);
        break;
    default:
        buff_.Append("[info]:", 9);
        break;
    }
}
int Log::GetLevel() {
    lock_guard<mutex> locker(mtx_);
    return level_;
}

void Log::Setlevel(int level) {
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}