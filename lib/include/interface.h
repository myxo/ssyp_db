#pragma once
#include <memory>
#include <string>

#include "settings.h"

class ITransaction {
};
using ITransactionPtr = std::unique_ptr<ITransaction>;

enum class CommitStatus {
    Success,
    Error
};

class ISsypDB {
public:
    virtual ITransactionPtr StartTransaction() = 0;

    virtual void SetValue(std::string key, std::string value, ITransactionPtr& tx) = 0;
    virtual bool GetValue(std::string key, str::string& value) = 0;

    virtual CommitStatus Commit(ITransactionPtr tx) = 0;
};

std::shared_ptr<ISsypDB> CreateDb(DbSettings settings);
