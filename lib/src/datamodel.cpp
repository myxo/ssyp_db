#include "datamodel.h"

#include <map>
#include <string>

class Datamodel : public IDatamodel {
public:
    Datamodel(IStoragePtr storage, DbSettings settings) {
        storage_ = storage;
        journal_ = storage->GetJournal();
        journal_limit_ = settings.journal_limit;
    }

    bool Commit(Operations ops) {
        std::vector<std::string> output = {};
        std::string temp;
        for (auto const& op : ops) {
            if (op.type == Op::Type::Remove) {
                temp = "Remove " + op.key;
            } else {
                temp = "Update " + op.key + " " + op.value;
            }
            temp += " " + std::to_string(op.key.size());
            output.push_back(temp);
            journal_.push_back(temp);
        }
        storage_->WriteToJournal(output);
        if (journal_.size() > journal_limit_) {
            storage_->PushJournalToTable(TableToString(GenerateTable()));
            journal_.clear();
        }
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        std::string s_type, s_key, s_value, key_length;
        int key_length_int;
        for (auto it = journal_.rbegin(); it != journal_.rend(); it++) {
            key_length = it->substr(it->find_last_of(' ') + 1);
            key_length_int = std::stoi(key_length);
            s_key = it->substr(it->find(' ') + 1, key_length_int);
            if (s_key == key) {
                s_type = it->substr(0, it->find(' '));
                if (s_type == "Update") {
                    value = it->substr(it->find(' ') + key_length_int + 2,
                                       it->find_last_of(' ') - it->find(' ') -
                                           key_length_int - 2);
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }

    std::map<std::string, std::string> GenerateTable() {
        std::vector<std::string> banned_keys;
        std::map<std::string, std::string> table;
        std::string s_key, s_type, s_value, key_length;
        int key_length_int;
        for (auto it = journal_.rbegin(); it != journal_.rend(); it++) {
            key_length = it->substr(it->find_last_of(' ') + 1);
            key_length_int = std::stoi(key_length);
            s_key = it->substr(it->find(' ') + 1, key_length_int);
            if ((table.find(s_key) == table.end()) &&
                (std::find(banned_keys.begin(), banned_keys.end(), s_key) ==
                 banned_keys.end())) {
                s_type = it->substr(0, it->find(' '));
                if (s_type == "Remove") {
                    banned_keys.push_back(s_key);
                } else {
                    s_value = it->substr(it->find(' ') + key_length_int + 2,
                                         it->find_last_of(' ') - it->find(' ') -
                                             key_length_int - 2);
                    table[s_key] = s_value;
                }
            }
        }
        return table;
    }

    std::string TableToString(std::map<std::string, std::string> table) {
        std::string s = "";
        for (auto it = table.begin(); it != table.end(); it++) {
            s += std::to_string(it->first.size()) + " " + it->first + " " +
                 std::to_string(it->second.size()) + " " + it->second + " ";
        }
        s.pop_back();
        return s;
    }

    std::map<std::string, std::string> StringToTable(std::string s) {
        int key_length, value_length;
        std::map<std::string, std::string> table;
        std::string s_key, s_value;
        while (s != "") {
            key_length = std::stoi(s.substr(0, s.find(' ')));
            s_key = s.substr(s.find(' ') + 1, key_length);
            value_length =
                std::stoi(s.substr(s.find(' ') + key_length + 2,
                                   s.find(' ', s.find(' ') + key_length + 2)));
            s_value = s.substr(s.find(' ', s.find(' ') + key_length + 2) + 1,
                               value_length);
            table[s_key] = s_value;
            s = s.substr(s.find(' ', s.find(' ') + key_length + 2) + 1 +
                         value_length);
            if (s != "") {
                s = s.substr(1);
            }
        }
        return table;
    }

private:
    IStoragePtr storage_;
    std::vector<std::string> journal_;
    int journal_limit_;
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    IDatamodelPtr output = std::make_shared<Datamodel>(storage, settings);
    return output;
}
