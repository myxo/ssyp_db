#include "logging.h"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

static LogLevel globalLevel = LogLevel::Info;
static std::vector<std::string> logsQueue;
static bool isLogThread = false;
static bool stopThread = false;
static std::thread logThread;
std::mutex mutex;

void SetLogLevel(LogLevel level) { globalLevel = level; }

void StopLogThread() { stopThread = 1; }

void FlushLog() {
    while (true) {
        std::unique_lock lock(mutex);
        if (logsQueue.empty()) {
            return;
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void LogThreadCycle() {
    while (!stopThread) {
        if (!logsQueue.empty()) {
            std::vector<std::string> localLogsQueue;
            {
                std::lock_guard<std::mutex> lock(mutex);
                std::swap(localLogsQueue, logsQueue);
            }

            for (auto it = localLogsQueue.begin(); it != localLogsQueue.end();
                 it++) {
                std::cout << *it << '\n';
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void InitializeLogThread() {
    if (isLogThread) {
        return;
    }
    logThread = std::thread(LogThreadCycle);
    logThread.detach();
    isLogThread = true;
}
// TODO: deinitialize

void Info(std::string input) {
    if (!isLogThread) {
        InitializeLogThread();
    }
    if (globalLevel == LogLevel::Info || globalLevel == LogLevel::Debug) {
        std::lock_guard<std::mutex> lock(mutex);
        logsQueue.push_back(input);
    }
}

void Error(std::string input) {
    if (!isLogThread) {
        InitializeLogThread();
    }
    if (globalLevel == LogLevel::Error || globalLevel == LogLevel::Info ||
        globalLevel == LogLevel::Debug) {
        std::lock_guard<std::mutex> lock(mutex);
        logsQueue.push_back(input);
    }
}

void Debug(std::string input) {
    if (!isLogThread) {
        InitializeLogThread();
    }
    if (globalLevel == LogLevel::Debug) {
        std::lock_guard<std::mutex> lock(mutex);
        logsQueue.push_back(input);
    }
}
