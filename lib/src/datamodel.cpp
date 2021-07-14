#include "datamodel.h"

#include <map>
#include <string>
#include <unordered_set>

class Datamodel : public IDatamodel {
public:
    Datamodel(IStoragePtr storage, DbSettings settings) {
        storage_ = storage;
        journal_ =
            std::make_shared<Operations>(JournalToOps(storage->GetJournal()));
        journal_limit_ = settings.journal_limit;
        table_limit_ = settings.table_limit;
        journal_->reserve(journal_limit_);
        counter = 0;
    }

    bool Commit(Operations ops) {
        std::vector<std::string> output = {};
        std::string temp;
        if (journal_->size() + ops.size() > journal_limit_) {
            storage_->PushJournalToTable(TableToString(GenerateTable()));
            journal_ = std::make_shared<Operations>();
            counter++;
            if (counter > table_limit_) {
                MergeTablesCheck();
                counter = 0;
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
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        auto l_journal = journal_;
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

private:
    IStoragePtr storage_;
    std::shared_ptr<Operations> journal_;
    size_t journal_limit_;
    size_t table_limit_;
    int counter;

    Operations JournalToOps(std::vector<std::string> s) {
        Operations ops;
        for (auto it = s.begin(); it < s.end(); it++) {
            ops.push_back(StringToOp(*it));
        }
        return ops;
    }

    Op StringToOp(std::string const& s) {
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
        std::map<std::string, std::string> table_;
        int key_length_int;
        for (auto it = journal_->rbegin(); it != journal_->rend(); it++) {
            if (table_.find(it->key) == table_.end()) {
                table_[it->key] = it->value;
            }
        }
        table.table_data = table_;
        table.table_level = 1;
        return table;
    }

    std::string TableToString(Table table) {
        std::map<std::string, std::string> table_ = table.table_data;
        std::string s;
        for (auto it = table_.begin(); it != table_.end(); it++) {
            s += std::to_string(it->first.size()) + " " + it->first +
                 std::to_string(it->second.size()) + " " + it->second;
        }
        s += " " + std::to_string(table.table_level);
        return s;
    }

    Table StringToTable(std::string s) {
        int length, i;
        Table table;
        std::map<std::string, std::string> table_;
        std::string value, s_level;
        s_level = s.substr(s.find_last_of(' ') + 1);
        table.table_level = std::stoi(s_level);
        i = 0;
        std::vector<std::string> values;
        while (i < s.find_last_of(' ')) {
            int space_pos = s.find(' ', i);
            length = std::stoi(s.substr(i, space_pos));
            value = s.substr(space_pos + 1, length);
            values.push_back(value);
            i = space_pos + 1 + length;
        }
        for (int j = 0; j < values.size(); j += 2) {
            table_[values[j]] = values[j + 1];
        }
        table.table_data = table_;
        return table;
    }

    void MergeTablesCheck() {
        std::map<int, std::vector<Table>> tables_for_level;
        std::vector<size_t> indexes_for_merge;
        for (int i = 0; i < storage_->GetTableList()->TableCount(); i++) {
            Table table_ = StringToTable(storage_->GetTableList()->GetTable(i));
            tables_for_level[table_.table_level].push_back(table_);
        }
        for (int i = 1; i < table_limit_; i++) {
            if (tables_for_level[i].size() > table_limit_) {
                indexes_for_merge.clear();
                Table merged_table = MergeTables(tables_for_level[i]);
                for (int j = 0; j < storage_->GetTableList()->TableCount();
                     j++) {
                    if (StringToTable(storage_->GetTableList()->GetTable(j))
                            .table_level == i) {
                        indexes_for_merge.push_back(j);
                    }
                }
                storage_->MergeTable(indexes_for_merge,
                                     TableToString(merged_table));
                tables_for_level[i + 1].push_back(merged_table);
            }
        }
    }

    Table MergeTables(std::vector<Table> tables) {
        Table table;
        table.table_level = tables[0].table_level + 1;
        for (auto it = tables.rbegin(); it < tables.rend(); it++) {
            std::map<std::string, std::string> data = it->table_data;
            for (auto const item : data) {
                if (table.table_data.find(item.first) ==
                    table.table_data.end()) {
                    table.table_data[item.first] = item.second;
                }
            }
        }
        return table;
    }
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    IDatamodelPtr output = std::make_shared<Datamodel>(storage, settings);
    return output;
}
