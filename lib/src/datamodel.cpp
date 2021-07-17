#include "datamodel.h"

#include <atomic>
#include <map>
#include <string>
#include <unordered_set>

#include "logging.h"

class Datamodel : public IDatamodel {
public:
    Datamodel(IStoragePtr storage, DbSettings settings) {
        storage_ = storage;
        journal_ =
            std::make_shared<Operations>(JournalToOps(storage->GetJournal()));
        journal_limit_ = settings.journal_limit;
        table_limit_ = settings.table_limit;
        journal_->reserve(journal_limit_);
    }

    bool Commit(Operations ops) {
        std::vector<std::string> output = {};
        std::string temp;
        if (journal_->size() + ops.size() > journal_limit_) {
            storage_->PushJournalToTable(TableToString(GenerateTable()));
            journal_ = std::make_shared<Operations>();
            table_write_counter_++;
            create_table_count_++;
            if (table_write_counter_ > table_limit_) {
                merge_tables_count_++;
                MergeTablesCheck();
                table_write_counter_ = 0;
            }
        }
        for (auto const& op : ops) {
            if (op.type == Op::Type::Remove) {
                temp = "Remove " + op.key;
            } else {
                temp = "Update " + op.key + " " + op.value;
            }
            temp += " " + std::to_string(op.key.size());
            output.push_back(temp);
            journal_->push_back(op);
        }
        storage_->WriteToJournal(output);
        commit_count_++;
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        auto l_journal = journal_;
        get_value_count_++;
        for (auto it = l_journal->rbegin(); it != l_journal->rend(); it++) {
            if (it->key == key) {
                if (it->type == Op::Type::Update) {
                    value = it->value;
                    return true;
                } else {
                    return false;
                }
            }
        }
        ITableListPtr tables = storage_->GetTableList();
        for (int i = tables->TableCount() - 1; i >= 0; i--) {
            std::map<std::string, std::string> table =
                StringToTable(tables->GetTable(i)).table_data;
            if (table.find(key) == table.end()) {
                return false;
            } else {
                if (table[key].size() == 0) {
                    return false;
                }
                value = table[key];
                return true;
            }
        }
        return false;
    }

    ~Datamodel() {
        Debug("DataModel Statistics:");
        Debug("\tCommits : " + std::to_string(commit_count_));
        Debug("\tGetValue: " + std::to_string(get_value_count_));
        Debug("\tTables created : " + std::to_string(create_table_count_));
        Debug("\tTable merges : " + std::to_string(merge_tables_count_));
    }

private:
    IStoragePtr storage_;
    std::shared_ptr<Operations> journal_;
    size_t journal_limit_;
    size_t table_limit_;
    int table_write_counter_ = 0;
    std::atomic_int commit_count_ = 0;
    std::atomic_int get_value_count_ = 0;
    std::atomic_int create_table_count_ = 0;
    std::atomic_int merge_tables_count_ = 0;

    Operations JournalToOps(std::vector<std::string> s) {
        Operations ops;
        for (auto it = s.begin(); it < s.end(); it++) {
            ops.push_back(StringToOp(*it));
        }
        return ops;
    }

    Op StringToOp(std::string const& s) const {
        Op op;
        int first_space = s.find(' ');
        int last_space = s.find_last_of(' ');
        int key_length = std::stoi(s.substr(last_space + 1));
        std::string type, key, value;
        type = s.substr(0, first_space);
        key = s.substr(first_space + 1, key_length);
        value = s.substr(first_space + key_length + 2,
                         last_space - first_space - key_length - 2);
        op.key = key;
        op.value = value;
        if (type == "Update") {
            op.type = Op::Type::Update;
        } else {
            op.type = Op::Type::Remove;
        }
        return op;
    }

    Table GenerateTable() {
        Table table;
        int key_length_int;
        for (auto it = journal_->rbegin(); it != journal_->rend(); it++) {
            if (table.table_data.find(it->key) == table.table_data.end()) {
                table.table_data[it->key] = it->value;
            }
        }
        table.table_level = 1;
        return table;
    }

    std::string TableToString(Table const& table) const {
        std::string s;
        for (auto const& [key, value] : table.table_data) {
            s += std::to_string(key.size()) + " " + key +
                 std::to_string(value.size()) + " " + value;
        }
        s += " " + std::to_string(table.table_level);
        return s;
    }

    Table StringToTable(std::string s) const {
        Table table;
        std::string value;
        int last_space = s.find_last_of(' ');
        std::string s_level = s.substr(last_space + 1);
        table.table_level = std::stoi(s_level);
        std::vector<std::string> values;
        for (int i = 0; i < last_space;) {
            int space_pos = s.find(' ', i);
            int length = std::stoi(s.substr(i, space_pos));
            value = s.substr(space_pos + 1, length);
            values.push_back(value);
            i = space_pos + 1 + length;
        }
        for (int j = 0; j < values.size(); j += 2) {
            table.table_data[values[j]] = values[j + 1];
        }
        return table;
    }

    void MergeTablesCheck() {
        std::map<int, std::vector<int>> indexes_for_level;
        std::vector<size_t> indexes_for_merge;
        ITableListPtr table_list = storage_->GetTableList();
        int table_count = table_list->TableCount();
        int merge_index = table_count - 1;
        int max_merge_level = 1;
        int prev_level = -1;
        int level_counter = 0;
        for (int i = table_count - 1; i >= 0; i--) {
            std::string s_table = table_list->GetTable(i);
            int level =
                std::stoi(s_table.substr(s_table.find_last_of(' ') + 1));
            if ((i == table_count - 1) || (level == prev_level)) {
                level_counter++;
            } else {
                if ((level_counter < table_limit_) || (level == table_limit_)) {
                    break;
                }
                level_counter = 1;
            }
            if (level_counter >= table_limit_) {
                max_merge_level = level;
                merge_index = i;
            }
            prev_level = level;
        }
        for (int i = merge_index; i < table_count; i++) {
            indexes_for_merge.push_back(i);
        }
        Table merged_table =
            MergeTables(indexes_for_merge, max_merge_level + 1);
        storage_->MergeTable(indexes_for_merge, TableToString(merged_table));
    }

    Table MergeTables(std::vector<size_t>& indexes, int merge_level) {
        std::vector<Table> tables;
        for (auto const it : indexes) {
            tables.push_back(
                StringToTable(storage_->GetTableList()->GetTable(it)));
        }
        Table out_table;
        out_table.table_level = merge_level;
        for (auto it = tables.rbegin(); it < tables.rend(); it++) {
            out_table.table_data.merge(it->table_data);
        }
        return out_table;
    }
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    IDatamodelPtr output = std::make_shared<Datamodel>(storage, settings);
    return output;
}
