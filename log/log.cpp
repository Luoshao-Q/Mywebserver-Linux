#include "log.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <mutex>

static std::ofstream g_logFile;
static std::mutex g_logMutex;

void initLog(const std::string& logFilePath) {
    g_logFile.open(logFilePath, std::ios::app);
    if (!g_logFile.is_open()) {
        std::cerr << "无法打开日志文件：" << logFilePath << std::endl;
    }
    logMessage("日志系统初始化完成");
}

std::string getCurrentTime() {
    time_t now = time(nullptr);
    char buf[64];
    ctime_r(&now, buf);
    std::string timeStr(buf);
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    return timeStr;
}

std::string levelToString(LogLevel level) {
    switch (level) {
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void logMessage(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(g_logMutex);

    std::string timeStr = getCurrentTime();
    std::string levelStr = levelToString(level);

    if (g_logFile.is_open()) {
        g_logFile << "[" << timeStr << "] [" << levelStr << "] " << message << std::endl;
        g_logFile.flush();
    }

    std::cout << "[" << timeStr << "] [" << levelStr << "] " << message << std::endl;
}

void closeLog() {
    if (g_logFile.is_open()) {
        logMessage("日志系统关闭");
        g_logFile.close();
    }
}