#pragma once
#include "storage.h"
#include "../include/settings.h"

#include <vector>
#include <memory>

struct Op {
    enum class Type {
        Update,
        Remove
    };
    std::string key;
    std::string value; // empty if type is Remove
    Type type;
};

using Operations = std::vector<Op>;

class IDatamodel {
public:
    // Write transaction data to datamodel
    virtual bool Commit(Operations ops) = 0;

    virtual bool GetValue(std::string key, std::string& value) = 0;
};
using IDatamodelPtr = std::shared_ptr<IDatamodel>;
IDatamodelPtr CreateDatamodel(IStoragePtr storage, DbSettings settings);
