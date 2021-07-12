#include "logging.h"

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

static LogLevel globalLevel = LogLevel::Info;
static std::vector<std::string> logsQueue;
static bool isLogThread = 0;
static bool stopThread = 0;
static std::thread logThread;
std::mutex mutex;

void SetLogLevel(LogLevel level) { globalLevel = level; }

void stopLogThread(){
	stopThread = 1;
}

void logThreadCycle(){
    while(!stopThread){
	    if (!logsQueue.empty()) {
	    	std::vector<std::string> localLogsQueue;
	    	{
			std::lock_guard<std::mutex> lock(mutex);
	        	std::swap(localLogsQueue, logsQueue);
	    	}

	    	for(auto it = localLogsQueue.begin(); it != localLogsQueue.end(); it++){
		   	 std::cout << *it + '\n';
	    	}
            }
	    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void initializeLogThread(){
    if (isLogThread){
        return;
    }
    logThread = std::thread(logThreadCycle);
    logThread.detach();
}
//TODO: deinitialize

void Info(std::string input) {
    if (!isLogThread){
        initializeLogThread();
    }
    if (globalLevel == LogLevel::Info || globalLevel == LogLevel::Debug) {
	std::lock_guard<std::mutex> lock(mutex);
	logsQueue.push_back(input);
    }
}

void Error(std::string input) {
    if (!isLogThread){
        initializeLogThread();
    }
    if (globalLevel == LogLevel::Error || globalLevel == LogLevel::Info ||
        globalLevel == LogLevel::Debug) {
	std::lock_guard<std::mutex> lock(mutex);
	logsQueue.push_back(input);
    }
}

void Debug(std::string input) {
    if (!isLogThread){
        initializeLogThread();
    }
    if (globalLevel == LogLevel::Debug) {
	std::lock_guard<std::mutex> lock(mutex);
	logsQueue.push_back(input);
    }
}
