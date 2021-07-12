#include "storage.h"

#include <cassert>
#include <fstream>

#include "in_memory_storage.h"

class TableList : public ITableList {
public:
    TableList(std::shared_ptr<std::string> filename, size_t* size)
        : filename_(filename), size_(size) {}
    size_t TableCount() const override { return *size_; }
    std::string GetTable(size_t index) const override {
        assert(index % *size_ == index);
        std::fstream file(*filename_);
        if (index > 0) {
            file.seekg(-sizeof(int32_t), std::ios_base::end);
            for (int i = *size_; i != index; i--) {
                int32_t previous_element;
                file.read((char*)&previous_element, sizeof(int32_t));
                file.seekg(previous_element);
            }
            file.seekg(sizeof(int), std::ios_base::cur);
        }
        std::string table;
        int32_t len;
        file.read((char*)&len, sizeof(int32_t));
        table.resize(len);
        file.read(table.data(), len);
        file.close();
        return table;
    }

private:
    std::shared_ptr<std::string> filename_;
    size_t* size_;
};

class Storage : public IStorage {
public:
    Storage(std::string filename)
        : journal_filename_(filename + ".journal"),
          tablelist_filename_(filename + ".tablelist") {
        size_of_tablelist_ = 0;
        std::ifstream tables_file(tablelist_filename_);
        if (!tables_file) {
            tables_file.close();
            return;
        }
        tables_file.seekg(-2 * sizeof(int32_t), std::ios::end);
        tables_file.read((char*)&size_of_tablelist_, sizeof(int32_t));
        tables_file.close();
    }
    bool WriteToJournal(std::vector<std::string> ops) override {
        std::ofstream file(journal_filename_, std::ios::app | std::ios::binary);
        if (!file) {
            throw std::runtime_error(std::string("Cannot open journal file: ") +
                                     journal_filename_);
            return false;
        }
        for (auto const it : ops) {
            int32_t tmp = std::hash<std::string>{}(it) % INT32_MAX;
            file.write((const char*)&tmp, sizeof(int32_t));
            tmp = (it.size());
            file.write((const char*)&tmp, sizeof(int32_t));
            file << it;
        }
        file.close();
        return true;
    }
    bool PushJournalToTable(std::string blob) override {
        int32_t last_element;
        {
            std::ifstream file(tablelist_filename_);
            file.seekg(-sizeof(int32_t), std::ios::end);
            last_element = size_of_tablelist_ == 0 ? -sizeof(int32_t)
                                                   : (int32_t)file.tellg();
            file.close();
        }
        std::ofstream file(tablelist_filename_,
                           std::ios::app | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open tablelist file: " +
                                     tablelist_filename_);
            return false;
        }
        int32_t len = blob.size();
        file.write((const char*)&len, sizeof(int32_t));
        file << blob;
        size_of_tablelist_++;
        file.write((const char*)&size_of_tablelist_, sizeof(int32_t));
        file.write((const char*)&last_element, sizeof(int32_t));
        file.close();
        std::remove(journal_filename_.c_str());
        return true;
    }
    ITableListPtr GetTableList() override {
        return std::make_shared<TableList>(
            std::make_shared<std::string>(tablelist_filename_),
            &size_of_tablelist_);
    }
    JournalBlob GetJournal() override {
        FILE* file = fopen(journal_filename_.c_str(), "rb");
        std::vector<std::string> journal;
        if (!file) {
            return {};
        }
        while (!(feof(file))) {
            int32_t hash;
            fread((void*)&hash, sizeof(int32_t), 1, file);
            int32_t len;
            fread((void*)&len, sizeof(int32_t), 1, file);
            if (feof(file)) break;
            journal.push_back({});
            journal.back().resize(len);
            fread(journal.back().data(), sizeof(char), len, file);
            assert(std::hash<std::string>{}(journal.back()) % INT32_MAX ==
                   hash);
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
    size_t size_of_tablelist_;
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
