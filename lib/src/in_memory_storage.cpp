#include "in_memory_storage.h"

InMemoryTableList::InMemoryTableList(
    std::shared_ptr<std::vector<std::string>> tables)
    : tables_(tables) {}

size_t InMemoryTableList::TableCount() const {
    return tables_->size();
}
std::string InMemoryTableList::GetTable(size_t index) const {
    return (*tables_).at(index);
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
bool InMemoryStorage::PushJournalToTable() {
    std::string blob;
    for (auto const& it : journal_) {
        blob += it;
    }
    journal_.resize(0);
    table_list_->push_back(blob);
    return true;
}
ITableListPtr InMemoryStorage::GetTableList() {
    return std::make_shared<InMemoryTableList>((table_list_));
}
JournalBlob InMemoryStorage::GetJournal() {
    return journal_;
}
