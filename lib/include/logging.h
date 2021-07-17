#pragma once

#include <string>

enum class LogLevel { Error, Info, Debug };

void SetLogLevel(LogLevel level);

void Info(std::string input);
void Debug(std::string input);
void Error(std::string input);

void StopLogThread();
void FlushLog();
