#include "logging.h"

#include <iostream>

static LogLevel globalLevel = LogLevel::Info;

void SetLogLevel(LogLevel level) { globalLevel = level; }

void Info(std::string input) {
    if (globalLevel == LogLevel::Info) {
        std::cout << input + '\n';
    }
}

void Error(std::string input) {
    if (globalLevel == LogLevel::Error) {
        std::cout << input + '\n';
    }
}

void Debug(std::string input) {
    if (globalLevel == LogLevel::Debug) {
        std::cout << input + '\n';
    }
}
