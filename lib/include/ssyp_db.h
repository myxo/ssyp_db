#pragma once

#include <memory>
#include <string>

#include "settings.h"

class ITransaction {};

using ITransactionPtr = std::shared_ptr<ITransaction>;

enum class CommitStatus { Success, Error };

class ISsypDB {
public:
    virtual ITransactionPtr StartTransaction() = 0;

    virtual void SetValue(std::string key, std::string value,
                          ITransactionPtr& tx) = 0;
    virtual void SetValue(std::string key, const char* value,
                          ITransactionPtr& tx) = 0;
    virtual void SetValue(std::string key, int value, ITransactionPtr& tx) = 0;
    virtual void SetValue(std::string key, double value,
                          ITransactionPtr& tx) = 0;
    virtual void SetValue(std::string key, bool value, ITransactionPtr& tx) = 0;

    virtual void Remove(std::string key, ITransactionPtr& tx) = 0;

    virtual bool GetValue(std::string key, std::string& value) = 0;
    virtual bool GetValue(std::string key, int& value) = 0;
    virtual bool GetValue(std::string key, double& value) = 0;
    virtual bool GetValue(std::string key, bool& value) = 0;

    virtual CommitStatus Commit(ITransactionPtr tx) = 0;
};

std::shared_ptr<ISsypDB> CreateDb(DbSettings settings);
