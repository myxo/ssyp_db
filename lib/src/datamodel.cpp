#include "datamodel.h"

#include <map>
#include <string>

class Datamodel : public IDatamodel {
public:
    bool Commit(Operations ops) {
        for (auto const& op : ops) {
            storage_[op.key] = op.value;
        }
        return true;
    }

    bool GetValue(std::string key, std::string& value) {
        auto it = storage_.find(key);
        if (it == storage_.end()) {
            return false;
        }
        value = it->second;
        return true;
    }

    bool LSMTCommit(Operations ops) {
        for (auto const& op : ops) {
            if (op.type == Op::Type::Remove) {
                storage_.erase(op.key);
            } else {
                storage_[op.key] = op.value;
            }
        }
        return true;
    }

    bool LSMTGetValue(std::string key, std::string& value) {
        for (auto it = storage_.rbegin(); it != storage_.rend(); it++) {
            if (it->first == key) {
                value = it->second;
                return true;
            }
        }
        return false;
    }

    void SerializeOps(Operations ops, IStorage& journal) {
        std::string temp;
        std::vector<std::string> output = {};
        for (auto const& op : ops) {
            if (op.type == Op::Type::Remove) {
                temp = "Remove " + op.key;
            } else {
                temp = "Update " + op.key + " " + op.value;
            }
            output.push_back(temp);
        }
        journal.WriteToJournal(output);
    }

private:
    std::map<std::string, std::string> storage_;
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    return std::make_shared<Datamodel>();
}
