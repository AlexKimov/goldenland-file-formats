#include "Logger.h"
#include "Settings.h"
#include <stdio.h>
#include <chrono>

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : hFile(INVALID_HANDLE_VALUE) {
    InitializeCriticalSection(&cs);
}

Logger::~Logger() {
    Close();
    DeleteCriticalSection(&cs);
}

void Logger::Init(const std::string& logFile) {
    EnterCriticalSection(&cs);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    hFile = CreateFileA(logFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    LeaveCriticalSection(&cs);
}

void Logger::Close() {
    EnterCriticalSection(&cs);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection(&cs);
}

void Logger::Write(const char* prefix, const char* fmt, va_list args) {
    if (hFile == INVALID_HANDLE_VALUE) return;

    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, args);

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
    localtime_s(&bt, &timer);

    char finalBuf[4200];
    sprintf_s(finalBuf, "[%02d:%02d:%02d.%03lld] %s%s\r\n", bt.tm_hour, bt.tm_min, bt.tm_sec, ms.count(), prefix, buf);

    EnterCriticalSection(&cs);
    DWORD written;
    WriteFile(hFile, finalBuf, (DWORD)strlen(finalBuf), &written, NULL);
    LeaveCriticalSection(&cs);
}

void Logger::Basic(const char* fmt, ...) {
    if (!Settings::GetInstance().BasicLogging) return;
    va_list args;
    va_start(args, fmt);
    Write("[BASIC] ", fmt, args);
    va_end(args);
}

void Logger::Extended(const char* fmt, ...) {
    if (!Settings::GetInstance().ExtendedLogging) return;
    va_list args;
    va_start(args, fmt);
    Write("[EXT] ", fmt, args);
    va_end(args);
}