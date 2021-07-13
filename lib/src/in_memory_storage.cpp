#include "in_memory_storage.h"

InMemoryTableList::InMemoryTableList(
    std::shared_ptr<std::vector<std::string>> tables,
    std::map<size_t, size_t> merged_tables_indexes)
    : tables_(tables), merged_tables_indexes_(merged_tables_indexes) {}

size_t InMemoryTableList::TableCount() const { return tables_->size(); }
std::string InMemoryTableList::GetTable(size_t index) const {
    size_t new_index = index;
    while (merged_tables_indexes_.count(new_index) == 1) {
        new_index = merged_tables_indexes_.at(new_index);
    }
    return tables_->at(new_index);
}

InMemoryStorage::InMemoryStorage() {
    table_list_ = std::make_shared<std::vector<std::string>>();
}
bool InMemoryStorage::WriteToJournal(std::vector<std::string> ops) {
    for (auto const& it : ops) {
        journal_.push_back(it);
    }
    return true;
}
bool InMemoryStorage::PushJournalToTable(std::string blob) {
    table_list_->push_back(blob);
    journal_.clear();
    return true;
}
ITableListPtr InMemoryStorage::GetTableList() {
    return std::make_shared<InMemoryTableList>(table_list_,
                                               merged_tables_indexes_);
}
JournalBlob InMemoryStorage::GetJournal() { return journal_; }

bool InMemoryStorage::MergeTable(std::vector<size_t> merged_tables,
                                 std::string result_table) {
    for (auto const it : merged_tables) {
        if (merged_tables_indexes_.count(it) == 0) {
            merged_tables_indexes_.insert({it, table_list_->size()});
        } else {
            return false;
        }
    }
    table_list_->push_back(result_table);
}
