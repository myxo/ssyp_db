#pragma once
#include <string>

struct DbSettings {
    bool in_memory = false;
    std::string filename;
    size_t journal_limit = 100;
    size_t table_limit = 4;
};