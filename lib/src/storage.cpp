#include "storage.h"

class TableListInMemory : public ITableList {
public:
    size_t TableCount() override {
        return table_count_;
    }
    std::string GetTable(size_t index) override {
        return tables_[index];
    }
    bool AddTable(std::string blob) override {
        tables_.push_back(blob);
        table_count_++;
        return true;
    }
private:
    size_t table_count_ = 0;
    std::vector<std::string> tables_;
};

class StorageInMemory : public IStorage {
    bool WriteToJournal(std::vector<std::string> ops) override {
        for (auto it : ops) {
            journal_.push_back(it);
        }
        return true;
    }
    bool AddTable(std::string blob) override {
        return table_list_.AddTable(blob);
    }
    ITableListPtr GetTableList() override {
        return std::make_shared<TableListInMemory>(table_list_);
    }
    JournalBlob GetJournal() override {
        return journal_;
    }
private:
    std::vector<std::string> journal_;
    TableListInMemory table_list_;
};

IStoragePtr CreateStorage(DbSettings settings) {
    if (settings.in_memory) {
        IStoragePtr storage = std::make_shared<StorageInMemory>();
        return storage;
    }
}