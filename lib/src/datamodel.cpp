#include "datamodel.h"

#include <map>
#include <string>
#include <unordered_set>

class Datamodel : public IDatamodel {
public:
    Datamodel(IStoragePtr storage, DbSettings settings) {
        storage_ = storage;
        journal_ = StringToOps(storage->GetJournal());
        journal_limit_ = settings.journal_limit;
    }

    bool Commit(Operations ops) {
        std::vector<std::string> output = {};
        std::string temp;
        if (journal_.size() + ops.size() > journal_limit_) {
            storage_->PushJournalToTable(TableToString(GenerateTable()));
            journal_.clear();
        }
        for (auto const& op : ops) {
            if (op.type == Op::Type::Remove) {
                temp = "Remove " + op.key;
            } else {
                temp = "Update " + op.key + " " + op.value;
            }
            temp += " " + std::to_string(op.key.size());
            output.push_back(temp);
            journal_.push_back(op);
        }
        storage_->WriteToJournal(output);
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        for (auto it = journal_.rbegin(); it != journal_.rend(); it++) {
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
                StringToTable(tables->GetTable(i));
            if (table.find(key) == table.end()) {
                return false;
            } else {
                value = table[key];
                return true;
            }
        }
        return false;
    }

    std::map<std::string, std::string> GenerateTable() {
        std::unordered_set<std::string> banned_keys;
        std::map<std::string, std::string> table;
        int key_length_int;
        for (auto it = journal_.rbegin(); it != journal_.rend(); it++) {
            if ((table.find(it->key) == table.end()) &&
                (std::find(banned_keys.begin(), banned_keys.end(), it->key) ==
                 banned_keys.end())) {
                if (it->type == Op::Type::Remove) {
                    banned_keys.insert(it->key);
                } else {
                    table[it->key] = it->value;
                }
            }
        }
        return table;
    }

    std::string TableToString(std::map<std::string, std::string> table) {
        std::string s;
        for (auto it = table.begin(); it != table.end(); it++) {
            s += std::to_string(it->first.size()) + " " + it->first +
                 std::to_string(it->second.size()) + " " + it->second;
        }
        return s;
    }

    std::map<std::string, std::string> StringToTable(std::string s) {
        int length, i;
        std::map<std::string, std::string> table;
        std::string value;
        i = 0;
        std::vector<std::string> values;
        while (i < s.size()) {
            int space_pos = s.find(' ', i);
            length = std::stoi(s.substr(i, space_pos));
            value = s.substr(space_pos + 1, length);
            values.push_back(value);
            i = space_pos + 1 + length;
        }
        for (int j = 0; j < values.size(); j += 2) {
            table[values[j]] = values[j + 1];
        }
        return table;
    }

    Operations StringToOps(std::vector<std::string> s) {
        Operations ops;
        for (auto it = s.begin(); it < s.end(); it++) {
            Op op;
            int first_space = it->find(' ');
            int last_space = it->find_last_of(' ');
            int key_length = std::stoi(it->substr(last_space + 1));
            std::string type, key, value;
            type = it->substr(0, first_space);
            key = it->substr(first_space + 1, key_length);
            value = it->substr(first_space + key_length + 2,
                               last_space - first_space - key_length - 2);
            op.key = key;
            op.value = value;
            if (type == "Update") {
                op.type = Op::Type::Update;
            } else {
                op.type = Op::Type::Remove;
            }
            ops.push_back(op);
        }
        return ops;
    }

private:
    IStoragePtr storage_;
    Operations journal_;
    int journal_limit_;
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    IDatamodelPtr output = std::make_shared<Datamodel>(storage, settings);
    return output;
}
