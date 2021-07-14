#pragma once
#include "storage.h"

class InMemoryTableList : public ITableList {
public:
    InMemoryTableList(std::vector<std::shared_ptr<std::string>> tables);
    size_t TableCount() const override;
    std::string GetTable(size_t index) const override;

private:
    std::vector<std::shared_ptr<std::string>> tables_;
};

class InMemoryStorage : public IStorage {
public:
    bool WriteToJournal(std::vector<std::string> ops) override;
    bool PushJournalToTable(std::string blob) override;
    ITableListPtr GetTableList() override;
    JournalBlob GetJournal() override;
    bool MergeTable(std::vector<size_t> merged_tables,
                    std::string result_table) override;

private:
    std::vector<std::string> journal_;
    std::vector<std::shared_ptr<std::string>> table_list_;
    std::atomic<bool> writing = false;
};
