#include "storage.h"

#include "in_memory_storage.h"

class Storage : public IStorage {
public:
    Storage(std::string journal_filename, std::string tablelist_filename)
        : journal_filename_(journal_filename + ".journal"),
          tablelist_filename_(tablelist_filename + ".tablelist") {}
    bool WriteToJournal(std::vector<std::string> ops) override {
        std::ofstream file;
        if (!file) {
            return false;
        }
        file.open(journal_filename_, std::ios::app);
        for (auto const& it : ops) {
            file << it << std::endl;
            journal_size_++;
        }
        file.close();
        return true;
    }
    bool PushJournalToTable(std::string blob) override { return false; }
    ITableListPtr GetTableList() override { return nullptr; }
    JournalBlob GetJournal() override {
        std::ifstream file;
        file.open(journal_filename_);
        std::vector<std::string> journal;
        if (!file) {
            return {"null"};
        }
        for (int i = 0; i < journal_size_; i++) {
            journal.resize(journal.size() + 1);
            file >> journal[journal.size() - 1];
        }
        file.close();
        return journal;
    }

private:
    std::shared_ptr<std::vector<std::string>> table_list_;
    std::string journal_filename_;
    size_t journal_size_ = 0;
    std::string tablelist_filename_;
};

IStoragePtr CreateStorage(DbSettings settings) {
    if (settings.in_memory) {
        IStoragePtr storage = std::make_shared<InMemoryStorage>();
        return storage;
    } else {
        IStoragePtr storage = std::make_shared<Storage>(
            settings.journal_filename, settings.tablelist_filename);
        return storage;
    }
}
