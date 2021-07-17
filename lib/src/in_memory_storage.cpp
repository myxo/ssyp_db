#include "in_memory_storage.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "logging.h"

InMemoryTableList::InMemoryTableList(
    std::vector<std::shared_ptr<std::string>> tables,
    std::atomic_int& read_table_count, std::atomic_bool& is_writting,
    std::atomic_bool& is_reading)
    : tables_(tables),
      read_table_count_(read_table_count),
      is_writting_(is_writting),
      is_reading_(is_reading) {}

size_t InMemoryTableList::TableCount() const {
    while (is_writting_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    is_reading_ = true;
    auto len = tables_.size();
    is_reading_ = false;
    return len;
}

std::string InMemoryTableList::GetTable(size_t index) const {
    while (is_writting_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    is_reading_ = true;
    read_table_count_++;
    auto table = *(tables_.at(index));
    is_reading_ = false;
    return table;
}

bool InMemoryStorage::WriteToJournal(std::vector<std::string> ops) {
    while (is_reading_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    is_writting_ = true;
    for (auto const& it : ops) {
        journal_.push_back(it);
    }
    statistic_.write_journal_count_++;
    is_writting_ = false;
    return true;
}
bool InMemoryStorage::PushJournalToTable(std::string blob) {
    while (is_reading_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    is_writting_ = true;
    table_list_.push_back(std::make_shared<std::string>(blob));
    journal_.clear();
    statistic_.push_table_count_++;
    is_writting_ = false;
    return true;
}
ITableListPtr InMemoryStorage::GetTableList() {
    while (is_writting_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    is_reading_ = true;
    auto table_list = std::make_shared<InMemoryTableList>(
        table_list_, statistic_.read_table_count_, is_writting_, is_reading_);
    is_reading_ = false;
    return table_list;
}
JournalBlob InMemoryStorage::GetJournal() {
    while (is_writting_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    statistic_.read_journal_count_++;
    return journal_;
}

bool InMemoryStorage::MergeTable(std::vector<size_t> merged_tables,
                                 std::string result_table) {
    std::sort(merged_tables.begin(), merged_tables.end(),
              std::greater<size_t>());
    while (is_reading_)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    is_writting_ = true;
    for (auto const it : merged_tables) {
        table_list_.erase(table_list_.begin() + it);
    }
    table_list_.push_back(std::make_shared<std::string>(result_table));
    statistic_.merge_table_count_++;
    is_writting_ = false;
    return true;
}
