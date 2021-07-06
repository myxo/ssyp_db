#include "in_memory_storage.h"

InMemoryTableList::InMemoryTableList(
    std::shared_ptr<std::vector<std::string>> tables) {
    tables_ = tables;
}
size_t InMemoryTableList::TableCount() {
    return tables_->size();
}
std::string InMemoryTableList::GetTable(size_t index) {
    return (*tables_)[index];
}

bool InMemoryStorage::WriteToJournal(std::vector<std::string> ops) {
    for (auto const& it : ops) {
        journal_.push_back(it);
    }
    return true;
}
bool InMemoryStorage::AddTable(std::string blob) {
    table_list_.push_back(blob);
    return true;
}
ITableListPtr InMemoryStorage::GetTableList() {
    InMemoryTableList tables(
        std::make_shared<std::vector<std::string>>(table_list_));
    return std::make_shared<InMemoryTableList>(tables);
}
JournalBlob InMemoryStorage::GetJournal() {
    return journal_;
}