#include "storage.h"

#include <fstream>

#include "in_memory_storage.h"

class Storage : public IStorage {
public:
    Storage(std::string filename)
        : journal_filename_(filename + ".journal"),
          tablelist_filename_(filename + ".tablelist") {}
    bool WriteToJournal(std::vector<std::string> ops) override {
        std::ofstream file(journal_filename_, std::ios::app);
        if (!file) {
            throw std::runtime_error(std::string("Cannot open journal file: ") +
                                     journal_filename_);
            return false;
        }
        for (auto const& it : ops) {
            file << it << '\0';
        }
        file.close();
        return true;
    }
    bool PushJournalToTable(std::string blob) override { return false; }
    ITableListPtr GetTableList() override { return nullptr; }
    JournalBlob GetJournal() override {
        std::ifstream file(journal_filename_);
        std::vector<std::string> journal;
        if (!file) {
            throw std::runtime_error("Cannot open journal file: " +
                                     journal_filename_);
        }
        while (!(file.eof())) {
            journal.resize(journal.size() + 1);
            std::getline(file, journal[journal.size() - 1], '\0');
        }
        file.close();
        journal.pop_back();
        return journal;
    }

private:
    std::shared_ptr<std::vector<std::string>> table_list_;
    std::string journal_filename_;
    std::string tablelist_filename_;
};

IStoragePtr CreateStorage(DbSettings settings) {
    if (settings.in_memory) {
        IStoragePtr storage = std::make_shared<InMemoryStorage>();
        return storage;
    } else {
        if (settings.filename == "") {
            throw std::runtime_error("Filename is empty");
        }
        return std::make_shared<Storage>(settings.filename);
    }
}
