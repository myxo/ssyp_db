#include "storage.h"

class StorageInMemory : public IStorage {
    bool WriteToJournal(std::vector<std::string> ops) override {
        for (auto it : ops) {
            journal.push_back(it);
        }
        return true;
    }
    bool AddTable(std::string blob) override { return false; }
    ITableListPtr GetTableList() override { return nullptr; }
    JournalBlob GetJournal() override { return journal; }

private:
    std::vector<std::string> journal;
};

IStoragePtr CreateStorage(DbSettings settings) {
    if (settings.in_memory) {
        IStoragePtr storage = std::make_shared<StorageInMemory>();
        return storage;
    }
}