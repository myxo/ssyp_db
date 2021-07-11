#include "logging.h"

#include <iostream>

static LogLevel globalLevel = LogLevel::Info;

void SetLogLevel(LogLevel level){
    globalLevel = level;
}

void Info(std::string input){
	if (globalLevel == LogLevel::Info){
		std::cout << input;
	}
}

void Error(std::string input){
	if (globalLevel == LogLevel::Error){
		std::cout << input;
	}
}

void Debug(std::string input){
	if (globalLevel == LogLevel::Debug){
		std::cout << input;
	}
}
