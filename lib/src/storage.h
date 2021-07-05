#pragma once
#include "common.h"
#include "../include/settings.h"

#include <memory>
#include <string>

class ITableList {
public:
    virtual size_t TableSize() = 0;
    virtual LSMTable GetTable(size_t index) = 0;
};
using ITableListPtr = std::shared_ptr<ITableList>;


class IStorage {
public:
    
    virtual bool WriteToJournal(std::string blob) = 0;

    virtual bool AddTable(std::string blob) = 0;

    virtual ITableListPtr GetTableList() = 0;

    virtual Journal GetJournal() = 0;
};
using IStoragePtr = std::shared_ptr<IStorage>;

IStoragePtr CreateStorage(DbSettings settings);