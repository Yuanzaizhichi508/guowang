#pragma once

#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <string>
#include <mutex>
#include <fstream>
#include <queue>
#include <condition_variable>
#include <thread>
#include <sstream>
#include <iostream>

#define BUFFER_SIZE 512

static inline std::string ConvertFormatString(const char* format, va_list args){
    char buffer[BUFFER_SIZE];

    std::vsnprintf(buffer, BUFFER_SIZE, format, args);

    return std::string(buffer);
};

static inline std::string FillString(const std::string& str, const char fillChar, const int width){
    int len = str.length();
    if(width <= len || fillChar == '\0' || fillChar == '\n'){
        return str;
    }
    else{
        std::string res(width - len, fillChar);
        res += str;
        return res;
    }
}

class TimeUtil
{
public:
    static uint64_t GetTimeStampMicrosecond(){
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp_microsecond = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        auto timestamp_microsecond_since_epoch = timestamp_microsecond.time_since_epoch();
        long long timestamp_uint64 = std::chrono::duration_cast<std::chrono::microseconds>(timestamp_microsecond_since_epoch).count();
        return timestamp_uint64;
    };

    static std::string GetFormatTimeStampMicrosecond(){
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();

        // 转换为时间戳
        auto timestamp = std::chrono::system_clock::to_time_t(now);

        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&timestamp);

        // 格式化本地时间为字符串
        char buffer[80];
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", local_time);

        // 显示带年月日的时间字符串
        return std::string(buffer) + "." + FillString(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000), '0', 6);
    };

    static std::string GetSimpleFormatTimeStampMillisecond(){
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();

        // 转换为时间戳
        auto timestamp = std::chrono::system_clock::to_time_t(now);

        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&timestamp);

        // 格式化本地时间为字符串
        char buffer[80];
        std::strftime(buffer, 80, "%Y%m%d-%H%M%S", local_time);

        // 显示带年月日的时间字符串
        return std::string(buffer) + "-" + FillString(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000), '0', 3);
    };

    static std::string ConvertFormatToTimeStampMicrosecond(std::chrono::system_clock::time_point time_point){
        // 转换为时间戳
        auto timestamp = std::chrono::system_clock::to_time_t(time_point);

        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&timestamp);

        // 格式化本地时间为字符串
        char buffer[80];
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", local_time);

        // 显示带年月日的时间字符串
        return std::string(buffer) + "." + FillString(std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(time_point.time_since_epoch()).count() % 1000000), '0', 6);
    };
};


#define DEFAULT_LOG_PATH "log/"
#ifdef DEBUG
    #define FLUSH_INTERVAL 1
#else
    #define FLUSH_INTERVAL 5
#endif

enum class LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

struct LogStruct
{
    LogLevel level;                             // 日志级别
    std::chrono::system_clock::time_point time; // 日志时间
    std::thread::id thread_id;                  // 线程id
    std::string filename;                       // 日志源文件文件名
    int line;                                   // 日志源文件行号
    std::string content;                        // 日志内容

    std::string GetLevelString(){
        switch(level){
            case LogLevel::LOG_DEBUG:
                return "DEBUG";
            case LogLevel::LOG_INFO:
                return "INFO";
            case LogLevel::LOG_WARNING:
                return "WARNING";
            case LogLevel::LOG_ERROR:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    };

    std::string GetBaseFileName(){
        auto pos = filename.find_last_of("/\\");
        if(pos == std::string::npos){
            return filename;
        }
        else{
            return filename.substr(pos + 1);
        }
    };
};

class AsyncLogger
{
private:
    std::ofstream m_file;                           // 日志文件
    bool m_is_running;                              // 日志线程是否在运行
    std::queue<LogStruct> m_log_queue;              // 日志队列
    std::thread m_log_thread;                       // 日志线程
    std::mutex m_mutex;                             // 互斥锁
    std::condition_variable m_condition_variable;   // 条件变量

private:
    // 日志线程函数
    void LogThread(){
        int counter = 0;

        // 循环写日志
        while(m_is_running){
            // 获取日志队列锁
            std::unique_lock<std::mutex> lock(m_mutex);
            // 阻塞等待条件变量
            m_condition_variable.wait(lock, [this]{return !m_log_queue.empty() || !m_is_running;});
            while(!m_log_queue.empty()){
                // 获取日志
                auto log = m_log_queue.front();
                m_log_queue.pop();
                // 已经获取到日志内容，可以放开日志队列锁，在此期间可以继续往日志队列中添加日志
                lock.unlock();
                // 写入日志
                std::ostringstream oss;
                oss << "[" << TimeUtil::ConvertFormatToTimeStampMicrosecond(log.time) << "]"
                    << "[" << log.thread_id << "]"
                    << "[" << log.GetLevelString() << "]"
                    << "[" << log.GetBaseFileName() << "(" << log.line << ")" << "] "
                    << log.content << std::endl;
#ifdef DEBUG
                std::cout << oss.str();
#endif
                m_file << oss.str();
                counter++;
                if(counter >= FLUSH_INTERVAL){
                    m_file.flush();
                    counter = 0;
                }
                // 上锁，为下一次获取日志准备
                lock.lock();
            }
        }
    };

public:
    AsyncLogger(const std::string filename)
        : m_file(filename)
        , m_is_running(true)
        , m_log_thread(&AsyncLogger::LogThread, this)
    {   

    }

    ~AsyncLogger(){
        m_is_running = false;
        m_condition_variable.notify_one();
        if (m_log_thread.joinable()) {
            m_file.flush();
            m_log_thread.join();
        }
    };

    void WriteLog(const LogStruct& log){
        // 获取日志队列锁
        std::lock_guard<std::mutex> lock(m_mutex);
        // 将日志放入队列
        m_log_queue.push(log);
        // 通知条件变量
        m_condition_variable.notify_one();
    };

};

// 全局日志
class Logger
{
private:
    AsyncLogger m_async_logger; // 异步日志

private:
    Logger()
    : m_async_logger(DEFAULT_LOG_PATH + TimeUtil::GetSimpleFormatTimeStampMillisecond() + ".log")
    {
        LogStruct log = {LogLevel::LOG_INFO, std::chrono::system_clock::now(), std::this_thread::get_id(), __FILE__, __LINE__, "LOGGER INIT"};
        m_async_logger.WriteLog(log);
    };

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Log(LogLevel level, const char* file, int line, const std::string& content){
        auto now = std::chrono::system_clock::now();
        auto thread_id = std::this_thread::get_id();
        LogStruct log = {level, now, thread_id, file, line, content};
        m_async_logger.WriteLog(log);
    };

public:
    static Logger& GetInstance(){
        static Logger instance;
        return instance;
    };

    static inline void Debug(const char* file, int line, const std::string& content){
#ifdef DEBUG
        GetInstance().Log(LogLevel::LOG_DEBUG, file, line, content);
#endif
    };

    static inline void Debug(const char* file, int line, const char* format, ...){
        va_list args;
        va_start(args, format);
        GetInstance().Log(LogLevel::LOG_DEBUG, file, line, ConvertFormatString(format, args));
        va_end(args);
    };
    
    static inline void Info(const char* file, int line, const std::string& content){
        GetInstance().Log(LogLevel::LOG_INFO, file, line, content);
    };

    static inline void Info(const char* file, int line, const char* format, ...){
        va_list args;
        va_start(args, format);
        GetInstance().Log(LogLevel::LOG_INFO, file, line, ConvertFormatString(format, args));
        va_end(args);
    };

    static inline void Warning(const char* file, int line, const std::string& content){
        GetInstance().Log(LogLevel::LOG_WARNING, file, line, content);
    };

    static inline void Warning(const char* file, int line, const char* format, ...){
        va_list args;
        va_start(args, format);
        GetInstance().Log(LogLevel::LOG_WARNING, file, line, ConvertFormatString(format, args));
        va_end(args);
    };

    static inline void Error(const char* file, int line, const std::string& content){
        GetInstance().Log(LogLevel::LOG_ERROR, file, line, content);
    };

    static inline void Error(const char* file, int line, const char* format, ...){
        va_list args;
        va_start(args, format);
        GetInstance().Log(LogLevel::LOG_ERROR, file, line, ConvertFormatString(format, args));
        va_end(args);
    };
};

#define Log_Info(msg) Logger::Info(__FILE__, __LINE__, msg);
#define Log_Debug(msg) Logger::Debug(__FILE__, __LINE__, msg);
#define Log_Warning(msg) Logger::Warning(__FILE__, __LINE__, msg);
#define Log_Error(msg) Logger::Error(__FILE__, __LINE__, msg);

#define CONSIST_INTERVAL 60 * 1000 * 1000

// 输出条目
struct OutputLogItem
{
    uint64_t timeStamp;
    std::string output;

    OutputLogItem(const uint64_t& timeStamp, const std::string& output) : 
        timeStamp(timeStamp), 
        output(output)
        {};

    OutputLogItem(){};
};

// 将输出记录并使用网络输出
class OutputLogger
{
private: 
    std::deque<OutputLogItem> outputDeque;
    std::mutex outputMutex;

    void Add(const std::string& output){
        std::unique_lock<std::mutex> lock(this->outputMutex);
        uint64_t timeStamp = TimeUtil::GetTimeStampMicrosecond();
        while(!this->outputDeque.empty() && this->outputDeque.front().timeStamp < timeStamp - (uint64_t)(CONSIST_INTERVAL)){
            this->outputDeque.pop_front();
        }
        outputDeque.emplace_back(timeStamp, output);
    };

    // 直接插入，适合特殊用途
    void Add(const std::string& output, const uint64_t timeStamp){
        std::unique_lock<std::mutex> lock(this->outputMutex);
        outputDeque.emplace_back(timeStamp, output);
    };

    std::vector<OutputLogItem> Get(){
        std::unique_lock<std::mutex> lock(this->outputMutex);
        std::vector<OutputLogItem> ret;
        for(auto it = this->outputDeque.begin(); it != this->outputDeque.end(); it++){
            ret.push_back(*it);
        }
        return ret;
    };

    void Clear(){
        std::unique_lock<std::mutex> lock(this->outputMutex);
        this->outputDeque.clear();
    }

public:
    static OutputLogger& GetInstance(){
        static OutputLogger instance;
        return instance;
    };

    static void AddOutput(const std::string& output){
        GetInstance().Add(output);
    };

    static void AddOutput(const char* format, ...){
        va_list args;
        va_start(args, format);
        std::string str = ConvertFormatString(format, args);
        GetInstance().Add(str);
        std::cout << str;
        va_end(args);
    };

    static void AddOutputSpec(const std::string& output, const uint64_t timeStamp){
        GetInstance().Add(output, timeStamp);
    };

    static std::vector<OutputLogItem> GetOutput(){
        return GetInstance().Get();
    };

    // 清除记录
    static void ClearOutput(){
        GetInstance().Clear();
    }
};