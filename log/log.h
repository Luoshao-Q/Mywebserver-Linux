#ifndef LOG_H
#define LOG_H

#include <string>

enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

void initLog(const std::string& logFilePath);
void logMessage(const std::string& message, LogLevel level = LOG_INFO);
void closeLog();

#endif