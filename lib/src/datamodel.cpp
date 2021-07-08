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
            output.push_back(temp);
            journal_.push_back(temp);
        }
        storage_->WriteToJournal(output);
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        for (auto it = journal_.rbegin(); it != journal_.rend(); it++) {
            if (it->substr(7, key.size()) == key) {
                value = it->substr(8 + key.size());
                return true;
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
