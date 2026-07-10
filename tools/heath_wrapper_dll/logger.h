#pragma once
#include <windows.h>
#include <string>

class Logger {
public:
    static Logger& GetInstance();

    void Init(const std::string& logFile);
    void Close();

    void Basic(const char* fmt, ...);
    void Extended(const char* fmt, ...);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Write(const char* prefix, const char* fmt, va_list args);

    HANDLE hFile;
    CRITICAL_SECTION cs;
};

// Макросы для удобства
#define LOG_BASIC(fmt, ...) Logger::GetInstance().Basic(fmt, ##__VA_ARGS__)
#define LOG_EXT(fmt, ...) Logger::GetInstance().Extended(fmt, ##__VA_ARGS__)