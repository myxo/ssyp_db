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

private:
    std::map<std::string, std::string> storage_;
};

IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings) {
    return std::make_shared<Datamodel>();
}
