#pragma once
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <format> // �����ʽ����
#include <sstream> // �����ַ�������
#include <chrono>
#include <iomanip>

/*
* ��־���ȼ�
* �̰߳�ȫ
* �ļ����
* �ɱ����ģ��
* ��ʽ�����
* �ַ����� �ļ���
* ʱ���
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
    const char* timestampFormat = "%Y-%m-%d %H:%M:%S"; // ʱ���ʽ���ַ���
    const char* filePath = 0; // ��־�ļ�·��

public:
    static void SetPriority(LogPriority priority_) {
        GetInstance().priority = priority_;
    }
    static LogPriority GetPriority()
    {
        return GetInstance().priority;
    }

    // �����ļ����
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
    //������־·��
    static const char* GetFilePath() {
        return GetInstance().filePath;
    }
    static bool IsFileOutputEnabled()
    {
        return GetInstance().file.is_open();
    }
    //������־ʱ���ʽ��
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
    //����
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
            // ��ȡ��ǰʱ��
            auto now = std::chrono::system_clock::now();
            // ����ǰʱ��ת��Ϊʱ���
            auto nowTime = std::chrono::system_clock::to_time_t(now);
            // ����һ��tm�ṹ����������ڴ洢����ʱ��
            std::tm localTime;
            // ��ʱ���ת��Ϊ����ʱ��
            // �̰߳�ȫ��
            localtime_s(&localTime, &nowTime);

            // ����һ�������������ڱ���������Դ
            std::scoped_lock lock(mutex);

            // ��ʽ����־����
            // ʹ��std::vformat������message��args��ʽ��Ϊһ���ַ���
            std::string formattedMessage = std::vformat(message, std::make_format_args(args...));


            // ����������־��Ŀ
            std::stringstream logMessage;
            logMessage << std::put_time(&localTime, timestampFormat) << "\t"
                << messagePriorityStr << formattedMessage <<  " _LINE_ :" << lineNumber << "\t"
                << sourceFilePath << "\n";

            // ���������̨
            std::cout << logMessage.str();

            // ������ļ�
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