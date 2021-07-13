#include "in_memory_storage.h"

#include <algorithm>

InMemoryTableList::InMemoryTableList(
    std::vector<std::shared_ptr<std::string>> tables)
    : tables_(tables) {}

size_t InMemoryTableList::TableCount() const { return tables_.size(); }

std::string InMemoryTableList::GetTable(size_t index) const {
    return *(tables_.at(index));
}

bool InMemoryStorage::WriteToJournal(std::vector<std::string> ops) {
    for (auto const& it : ops) {
        journal_.push_back(it);
    }
    return true;
}
bool InMemoryStorage::PushJournalToTable(std::string blob) {
    table_list_.push_back(std::make_shared<std::string>(blob));
    journal_.clear();
    return true;
}
ITableListPtr InMemoryStorage::GetTableList() {
    return std::make_shared<InMemoryTableList>(table_list_);
}
JournalBlob InMemoryStorage::GetJournal() { return journal_; }

bool InMemoryStorage::MergeTable(std::vector<size_t> merged_tables,
                                 std::string result_table) {
    std::sort(merged_tables.begin(), merged_tables.end(),
              std::greater<size_t>());
    for (auto const it : merged_tables) {
        table_list_.erase(table_list_.begin() + it);
    }
    table_list_.push_back(std::make_shared<std::string>(result_table));
    return true;
}
