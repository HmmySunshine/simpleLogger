#pragma once
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <format> // 引入格式化库
#include <sstream> // 引入字符串流库
#include <chrono>
#include <iomanip>

/*
* 日志优先级
* 线程安全
* 文件输出
* 可变参数模板
* 格式化输出
* 字符串流 文件流
* 时间库
*/


class Logger {
public:
    enum LogPriority {
        TracePriority,
        DebugPriority,
        InfoPriority,
        WarnPriority,
        ErrorPriority,
        CriticalPriority,
    };
private:
    LogPriority priority = InfoPriority;
    std::mutex mutex;
    std::ofstream file;
    const char* timestampFormat = "%Y-%m-%d %H:%M:%S"; // 时间格式化字符串
    const char* filePath = 0; // 日志文件路径

public:
    static void SetPriority(LogPriority priority_) {
        GetInstance().priority = priority_;
    }
    static LogPriority GetPriority()
    {
        return GetInstance().priority;
    }

    // 启用文件输出
    static bool EnableFileOutPut() {
        Logger& instance = GetInstance();
        instance.filePath = "log.txt";
        return instance.EnableFileOut();
    }

    static bool EnableFileOutPut(const char* newFilePath) {
        Logger& instance = GetInstance();
        instance.filePath = newFilePath;
        return  instance.EnableFileOut();
    }
    //返回日志路径
    static const char* GetFilePath() {
        return GetInstance().filePath;
    }
    static bool IsFileOutputEnabled()
    {
        return GetInstance().file.is_open();
    }
    //设置日志时间格式化
    static void SetTimestampFormat(const char* newTimeStampFormat)
    {
        GetInstance().timestampFormat = newTimeStampFormat;
    }
    static const char* GetTimestampFormat()
    {
        return GetInstance().timestampFormat;
    }
    template<typename... Args>
    static void Trace(int line, const char* source,const char* message, Args... args) {
        GetInstance().Log(line, source, "[Trace]\t", TracePriority, message, args...);
    }

    template<typename... Args>
    static void Debug(int line, const char* source, const char* message, Args... args) {
        GetInstance().Log(line, source, "[Debug]\t", DebugPriority, message, args...);
    }

    template<typename... Args>
    static void Info(int line, const char* source, const char* message, Args... args) {
        GetInstance().Log(line, source,"[Info]\t", InfoPriority, message, args...);
    }

    template<typename... Args>
    static void Error(int line, const char* source, const char* message, Args... args) {
        GetInstance().Log(line, source, "[Error]\t", ErrorPriority, message, args...);
    }

    template<typename... Args>
    static void Warn(int line, const char* source, const char* message, Args... args) {
        GetInstance().Log(line, source, "[Warn]\t", WarnPriority, message, args...);
    }

    template<typename... Args>
    static void Critical(int line, const char* source, const char* message, Args... args) {
        GetInstance().Log(line, source, "[Critical]\t", CriticalPriority, message, args...);
    }

private:
    //单例
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    static Logger& GetInstance()
    {
        static Logger instance;
        return instance;
    }
    ~Logger()
    {
        FreeFile();
    }
    template<typename... Args>
    void Log(int lineNumber, const char* sourceFilePath,
        const char* messagePriorityStr, LogPriority logPriority,
        const char* message, Args... args) {
        if (priority <= logPriority) {
            // 获取当前时间
            auto now = std::chrono::system_clock::now();
            // 将当前时间转换为时间戳
            auto nowTime = std::chrono::system_clock::to_time_t(now);
            // 定义一个tm结构体变量，用于存储本地时间
            std::tm localTime;
            // 将时间戳转换为本地时间
            // 线程安全的
            localtime_s(&localTime, &nowTime);

            // 创建一个互斥锁，用于保护共享资源
            std::scoped_lock lock(mutex);

            // 格式化日志内容
            // 使用std::vformat函数将message和args格式化为一个字符串
            std::string formattedMessage = std::vformat(message, std::make_format_args(args...));


            // 构造完整日志条目
            std::stringstream logMessage;
            logMessage << std::put_time(&localTime, timestampFormat) << "\t"
                << messagePriorityStr << formattedMessage <<  " _LINE_ :" << lineNumber << "\t"
                << sourceFilePath << "\n";

            // 输出到控制台
            std::cout << logMessage.str();

            // 输出到文件
            if (file.is_open()) {
                file << logMessage.str();
            }
        }
    }

    bool EnableFileOut() {
        if (file.is_open()) {
            std::cout << "File is already opened" << std::endl;
            file.close();

        }
        file.open(filePath, std::ios::app);
        if (!file.is_open()) {
            std::cout << std::format("Failed to open file: {}", filePath) << std::endl;
            return false;
        }
        return true;
    }

    void FreeFile() {
        if (file.is_open()) {
            file.close();
        }
    }
};

#define LOG_TRACE(message, ...)(Logger::Trace(__LINE__, __FILE__, message, __VA_ARGS__))
#define LOG_INFO(message, ...)(Logger::Info(__LINE__, __FILE__, message, __VA_ARGS__))
#define Log_WARN(message, ...)(Logger::Warn(__LINE__, __FILE__, message, __VA_ARGS__))
#define LOG_ERROR(message, ...)(Logger::Error(__LINE__, __FILE__, message, __VA_ARGS__))
#define LOG_FATAL(message, ...)(Logger::Critical(__LINE__, __FILE__, message, __VA_ARGS__))
#define LOG_DEBUG(message, ...)(Logger::Debug(__LINE__, __FILE__, message, __VA_ARGS__))