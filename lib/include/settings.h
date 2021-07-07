#pragma once
#include <string>

struct DbSettings {
    bool in_memory = false;
    std::string journal_filename = "journal";
    std::string tablelist_filename = "tablelist";
};