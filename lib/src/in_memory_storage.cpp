#include "in_memory_storage.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "logging.h"

InMemoryTableList::InMemoryTableList(
    std::vector<std::shared_ptr<std::string>> tables,
    std::atomic_int& read_table_count, std::shared_ptr<std::mutex> mtx)
    : tables_(tables), read_table_count_(read_table_count), mutex_(mtx) {}

size_t InMemoryTableList::TableCount() const {
    std::lock_guard<std::mutex> guard(*mutex_);
    return tables_.size();
}

std::string InMemoryTableList::GetTable(size_t index) const {
    read_table_count_++;
    std::lock_guard<std::mutex> guard(*mutex_);
    return *(tables_.at(index));
}

bool InMemoryStorage::WriteToJournal(std::vector<std::string> ops) {
    statistic_.write_journal_count_++;
    std::lock_guard<std::mutex> guard(*mutex_);
    for (auto const& it : ops) {
        journal_.push_back(it);
    }
    return true;
}
bool InMemoryStorage::PushJournalToTable(std::string blob) {
    statistic_.push_table_count_++;
    std::lock_guard<std::mutex> guard(*mutex_);
    table_list_.push_back(std::make_shared<std::string>(blob));
    journal_.clear();
    return true;
}
ITableListPtr InMemoryStorage::GetTableList() {
    std::lock_guard<std::mutex> guard(*mutex_);
    return std::make_shared<InMemoryTableList>(
        table_list_, statistic_.read_table_count_, mutex_);
}
JournalBlob InMemoryStorage::GetJournal() {
    statistic_.read_journal_count_++;
    return journal_;
}

bool InMemoryStorage::MergeTable(std::vector<size_t> merged_tables,
                                 std::string result_table) {
    std::sort(merged_tables.begin(), merged_tables.end(),
              std::greater<size_t>());
    statistic_.merge_table_count_++;
    std::lock_guard<std::mutex> guard(*mutex_);
    for (auto const it : merged_tables) {
        table_list_.erase(table_list_.begin() + it);
    }
    table_list_.push_back(std::make_shared<std::string>(result_table));
    return true;
}
