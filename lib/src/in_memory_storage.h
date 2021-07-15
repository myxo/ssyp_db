#pragma once
#include "storage.h"

class InMemoryTableList : public ITableList {
public:
    InMemoryTableList(std::vector<std::shared_ptr<std::string>> tables,
                      std::shared_ptr<int> read_table_count);
    size_t TableCount() const override;
    std::string GetTable(size_t index) const override;

private:
    std::vector<std::shared_ptr<std::string>> tables_;
    std::shared_ptr<int> read_table_count_;
};

class InMemoryStorage : public IStorage {
public:
    bool WriteToJournal(std::vector<std::string> ops) override;
    bool PushJournalToTable(std::string blob) override;
    ITableListPtr GetTableList() override;
    JournalBlob GetJournal() override;
    bool MergeTable(std::vector<size_t> merged_tables,
                    std::string result_table) override;
    ~InMemoryStorage();

private:
    std::vector<std::string> journal_;
    std::vector<std::shared_ptr<std::string>> table_list_;

    int write_journal_count_ = 0;
    int read_journal_count_ = 0;
    int push_table_count_ = 0;
    int merge_table_count_ = 0;
    std::shared_ptr<int> read_table_count_ = std::make_shared<int>(0);
};
