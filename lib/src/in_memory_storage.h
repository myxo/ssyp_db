#pragma once
#include "storage.h"

class InMemoryTableList : public ITableList {
public:
    InMemoryTableList(std::shared_ptr<std::vector<std::string>> tables);
    const size_t TableCount() override;
    const std::string GetTable(size_t index) override;

private:
    std::shared_ptr<std::vector<std::string>> tables_;
};

class InMemoryStorage : public IStorage {
public:
    InMemoryStorage();
    bool WriteToJournal(std::vector<std::string> ops) override;
    bool AddTable(std::string blob) override;
    ITableListPtr GetTableList() override;
    JournalBlob GetJournal() override;
private:
    std::vector<std::string> journal_;
    std::shared_ptr<std::vector<std::string>> table_list_;
};
