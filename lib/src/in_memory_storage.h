#pragma once
#include "storage.h"

class InMemoryTableList : public ITableList {
public:
    InMemoryTableList(std::shared_ptr<std::vector<std::string>> tables);
    size_t TableCount() const override;
    std::string GetTable(size_t index) const override;

private:
    std::shared_ptr<std::vector<std::string>> tables_;
};

class InMemoryStorage : public IStorage {
public:
    InMemoryStorage();
    bool WriteToJournal(std::vector<std::string> ops) override;
    bool PushJournalToTable(std::string blob) override;
    ITableListPtr GetTableList() override;
    JournalBlob GetJournal() override;

private:
    std::vector<std::string> journal_;
    std::shared_ptr<std::vector<std::string>> table_list_;
};
