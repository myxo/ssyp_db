#pragma once
#include <string>

struct DbSettings {
    bool in_memory = false;
    std::string filename;
    int journal_limit;
};