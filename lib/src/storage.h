#pragma once
#include "../include/settings.h"
#include <memory>
#include <string>
#include <vector>
//#include "datamodel.h"

class ITableList {
public:
    virtual size_t TableCount() = 0;

    // Возвращает таблицу в виде бинарного блоба.
    // TODO: возвращать интерфейс для чтения из файла
    virtual std::string GetTable(size_t index) = 0;
};
using ITableListPtr = std::shared_ptr<ITableList>;

using JournalBlob = std::vector<std::string>;

class IStorage {
public:
    // Записывает операции в журнал. Каждый элемент вектора - отдельная операция.
    virtual bool WriteToJournal(std::vector<std::string> ops) = 0;

    // Записывает новую таблицу в базу
    virtual bool AddTable(std::string blob) = 0;

    // Возвращает view интерфейс на текущие таблицы
    virtual ITableListPtr GetTableList() = 0;

    // Возвращает журнал в виде бинарных данных (может быть полезно при восстановлении базы)
    virtual JournalBlob GetJournal() = 0;
};
using IStoragePtr = std::shared_ptr<IStorage>;

IStoragePtr CreateStorage(DbSettings settings);