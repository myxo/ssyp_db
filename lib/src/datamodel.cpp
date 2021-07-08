#include "datamodel.h"

#include <map>
#include <string>

class Datamodel : public IDatamodel {
public:
    Datamodel(IStoragePtr storage, DbSettings settings) {
        storage_ = storage;
        journal_ = storage->GetJournal();
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

private:
    IStoragePtr storage_;
    std::vector<std::string> journal_;
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    IDatamodelPtr output = std::make_shared<Datamodel>(storage, settings);
    return output;
}
