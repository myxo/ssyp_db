#include "datamodel.h"

#include <map>
#include <string>

class Datamodel : public IDatamodel {
public:
    Datamodel(IStoragePtr storage, DbSettings settings) { storage_ = storage; }

    bool Commit(Operations ops) {
        std::vector<std::string> output = {};
        std::string temp;
        for (auto const& op : ops) {
            if (op.type == Op::Type::Remove) {
                temp = "Remove " + op.key;
            } else {
                temp = "Update " + op.key + " " + op.value;
            }
            output.push_back(temp);
        }
        storage_->WriteToJournal(output);
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        std::string s_type, s_key, s_value;
        std::vector<std::string> journal_ = storage_->GetJournal();
        for (auto it = journal_.rbegin(); it != journal_.rend(); it++) {
            s_key = it->substr(it->find(' ') + 1,
                               it->find_last_of(' ') - it->find(' ') - 1);
            if (s_key == key) {
                s_type = it->substr(0, it->find(' '));
                if (s_type == "Update") {
                    value = it->substr(it->find_last_of(' ') + 1);
                    return true;
                } else {
                    value = "no_value";
                    return false;
                }
            }
        }
        value = "no_value";
        return false;
    }

private:
    IStoragePtr storage_;
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    IDatamodelPtr output = std::make_shared<Datamodel>(storage, settings);
    return output;
}
