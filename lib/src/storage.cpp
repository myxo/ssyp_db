#include "storage.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <set>

#include "in_memory_storage.h"
#include "logging.h"

class TableList : public ITableList {
public:
    TableList(std::string filename, std::vector<int64_t> table_addresses,
              std::atomic_int& read_table_count)
        : filename_(filename),
          table_addresses_(table_addresses),
          read_table_count_(read_table_count) {}
    size_t TableCount() const override { return table_addresses_.size(); }
    std::string GetTable(size_t index) const override {
        std::fstream file(filename_);
        file.seekg(table_addresses_.at(index) + sizeof(int64_t), std::ios::beg);
        int64_t merged_table_arr_len;
        file.read((char*)&merged_table_arr_len, sizeof(int64_t));
        file.seekg(merged_table_arr_len * sizeof(int64_t), std::ios::cur);
        std::string table;
        int64_t len;
        file.read((char*)&len, sizeof(int64_t));
        table.resize(len);
        file.read(table.data(), len);
        file.close();
        read_table_count_++;
        return table;
    }

private:
    std::string filename_;
    std::vector<int64_t> table_addresses_;
    std::atomic_int& read_table_count_;
};

class Storage : public IStorage {
public:
    Storage(std::string filename)
        : journal_filename_(filename + ".journal"),
          tablelist_filename_(filename + ".tablelist") {
        std::ifstream tables_file(tablelist_filename_);
        if (!tables_file) {
            tables_file.close();
            return;
        }
        int64_t previous_table;
        std::set<int64_t> merged_tables;
        tables_file.seekg(-1 * sizeof(int64_t), std::ios::end);
        tables_file.read((char*)&previous_table, sizeof(int64_t));
        while (previous_table > 0) {
            if (merged_tables.count(previous_table) == 0)
                table_addresses_.push_back(previous_table);
            tables_file.seekg(previous_table, std::ios::beg);
            tables_file.read((char*)&previous_table, sizeof(int64_t));
            int64_t len;
            tables_file.read((char*)&len, sizeof(int64_t));
            for (int i = 0; i < len; i++) {
                int64_t merged_table;
                tables_file.read((char*)&merged_table, sizeof(int64_t));
                merged_tables.insert(merged_table);
            }
        }
        if (merged_tables.count(-sizeof(int64_t)) == 0)
            table_addresses_.push_back(-sizeof(int64_t));
        std::reverse(std::begin(table_addresses_), std::end(table_addresses_));
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
            uint32_t tmp = std::hash<std::string>{}(it) %
                           std::numeric_limits<std::uint32_t>::max();
            file.write((const char*)&tmp, sizeof(uint32_t));
            tmp = (it.size());
            file.write((const char*)&tmp, sizeof(int32_t));
            file << it;
        }
        file.close();
        statistic_.write_journal_count_++;
        return true;
    }
    bool PushJournalToTable(std::string blob) override {
        int64_t last_element;
        {
            std::ifstream file(tablelist_filename_);
            file.seekg(-sizeof(int64_t), std::ios::end);
            last_element = table_addresses_.size() == 0 ? -sizeof(int64_t)
                                                        : (int64_t)file.tellg();
            file.close();
        }
        std::ofstream file(tablelist_filename_,
                           std::ios::app | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open tablelist file: " +
                                     tablelist_filename_);
            return false;
        }
        int64_t len = blob.size();
        int64_t merged_tables_len = 0;
        file.write((const char*)&merged_tables_len, sizeof(int64_t));
        file.write((const char*)&len, sizeof(int64_t));
        file << blob;
        table_addresses_.push_back(last_element);
        file.write((const char*)&last_element, sizeof(int64_t));
        file.close();
        std::remove(journal_filename_.c_str());
        statistic_.push_table_count_++;
        return true;
    }
    ITableListPtr GetTableList() override {
        return std::make_shared<TableList>(tablelist_filename_,
                                           table_addresses_,
                                           statistic_.read_table_count_);
    }
    JournalBlob GetJournal() override {
        FILE* file = fopen(journal_filename_.c_str(), "rb");
        std::vector<std::string> journal;
        if (!file) {
            return {};
        }
        bool is_journal_damaged = false;
        while (!(feof(file))) {
            uint32_t hash;
            fread((void*)&hash, sizeof(uint32_t), 1, file);
            int32_t len;
            fread((void*)&len, sizeof(int32_t), 1, file);
            if (feof(file)) break;
            journal.push_back({});
            journal.back().resize(len);
            fread(journal.back().data(), sizeof(char), len, file);
            if (std::hash<std::string>{}(journal.back()) %
                    std::numeric_limits<std::uint32_t>::max() ==
                hash) {
                if (is_journal_damaged) {
                    throw std::runtime_error("The journal is damaged");
                    break;
                }
                if (feof(file)) {
                    journal.pop_back();
                    break;
                }
            } else {
                journal.pop_back();
                is_journal_damaged = true;
            }
        }
        fclose(file);
        statistic_.read_journal_count_++;
        return journal;
    }
    bool MergeTable(std::vector<size_t> merged_tables,
                    std::string result_table) {
        std::sort(merged_tables.begin(), merged_tables.end(),
                  std::greater<size_t>());
        int64_t last_element;
        {
            std::ifstream file(tablelist_filename_);
            file.seekg(-sizeof(int64_t), std::ios::end);
            last_element = table_addresses_.size() == 0 ? -sizeof(int64_t)
                                                        : (int64_t)file.tellg();
            file.close();
        }
        std::ofstream file(tablelist_filename_,
                           std::ios::app | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open tablelist file: " +
                                     tablelist_filename_);
            return false;
        }
        int64_t len = result_table.size();
        int64_t merged_tables_len = merged_tables.size();
        file.write((const char*)&merged_tables_len, sizeof(int64_t));
        for (int64_t it : merged_tables) {
            file.write((const char*)&(table_addresses_.at(it)),
                       sizeof(int64_t));
            table_addresses_.erase(table_addresses_.begin() + it);
        }
        file.write((const char*)&len, sizeof(int64_t));
        file << result_table;
        table_addresses_.push_back(last_element);
        file.write((const char*)&last_element, sizeof(int64_t));
        file.close();
        statistic_.merge_table_count_++;
        return false;
    }

private:
    std::string journal_filename_;
    std::string tablelist_filename_;
    std::vector<int64_t> table_addresses_;
    StorageStatistic statistic_;
};

StorageStatistic::~StorageStatistic() {
    Debug("Storage statistic");
    Debug("\tJournal writing:\t" + std::to_string(write_journal_count_.load()));
    Debug("\tjournal readings:\t" + std::to_string(read_journal_count_.load()));
    Debug("\ttable pushings:\t\t" + std::to_string(push_table_count_.load()));
    Debug("\ttable mergings:\t\t" + std::to_string(merge_table_count_.load()));
    Debug("\ttable readings:\t\t" + std::to_string(read_table_count_.load()));
    Debug("\n");
}

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
