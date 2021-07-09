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
            int32_t tmp = (it.size());
            file.write((const char*)&tmp, sizeof(int32_t));
            file << it;
        }
        file.close();
        return true;
    }
    bool PushJournalToTable(std::string blob) override { return false; }
    ITableListPtr GetTableList() override { return nullptr; }
    JournalBlob GetJournal() override {
        FILE* file = fopen(journal_filename_.c_str(), "rb");
        std::vector<std::string> journal;
        if (!file) {
            throw std::runtime_error("Cannot open journal file: " +
                                     journal_filename_);
        }
        while (!(feof(file))) {
            int32_t len;
            fread((void*)&len, sizeof(int32_t), 1, file);
            if (feof(file)) break;
            journal.push_back({});
            journal.back().resize(len);
            fread(journal.back().data(), sizeof(char), len, file);
            if (feof(file)) {
                journal.pop_back();
                break;
            }
        }
        fclose(file);
        return journal;
    }

private:
    std::shared_ptr<std::vector<std::string>> table_list_;
    std::string journal_filename_;
    std::string tablelist_filename_;
};

IStoragePtr CreateStorage(DbSettings settings) {
    if (settings.in_memory) {
        return std::make_shared<InMemoryStorage>();
    } else {
        if (settings.filename == "") {
            throw std::runtime_error("Filename is empty");
        }
        return std::make_shared<Storage>(settings.filename);
    }
}
