#include "../include/ssyp_db.h"

#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

#include "datamodel.h"

class Transaction : public ITransaction {
public:
    Operations ops;
};

class SsypDB : public ISsypDB {
public:
    SsypDB(DbSettings settings)
        : datamodel_(CreateDatamodel(CreateStorage(settings), settings)) {}
    ITransactionPtr StartTransaction() override {
        return std::make_shared<Transaction>();
    }

    void SetValue(std::string key, std::string value,
                  ITransactionPtr& tx) override {
        ((Transaction*)(tx.get()))
            ->ops.push_back({key, value, Op::Type::Update});
    }
    void SetValue(std::string key, const char* value,
                  ITransactionPtr& tx) override {
        ((Transaction*)(tx.get()))
            ->ops.push_back({key, value, Op::Type::Update});
    }
    void SetValue(std::string key, int value, ITransactionPtr& tx) override {
        ((Transaction*)(tx.get()))
            ->ops.push_back({key, std::to_string(value), Op::Type::Update});
    }
    void SetValue(std::string key, double value, ITransactionPtr& tx) override {
        ((Transaction*)(tx.get()))
            ->ops.push_back({key, std::to_string(value), Op::Type::Update});
    }
    void SetValue(std::string key, bool value, ITransactionPtr& tx) override {
        ((Transaction*)(tx.get()))
            ->ops.push_back({key, value ? "true" : "false", Op::Type::Update});
    }

    void Remove(std::string key, ITransactionPtr& tx) override {
        ((Transaction*)(tx.get()))->ops.push_back({key, "", Op::Type::Remove});
    }

    bool GetValue(std::string key, std::string& value) override {
        return datamodel_->GetValue(key, value);
    }
    bool GetValue(std::string key, int& value) override {
        std::string tmp;
        datamodel_->GetValue(key, tmp);
        value = std::stoi(tmp);
        return true;
    }
    bool GetValue(std::string key, double& value) override {
        std::string tmp;
        datamodel_->GetValue(key, tmp);
        value = std::stod(tmp);
        return true;
    }
    bool GetValue(std::string key, bool& value) override {
        std::string tmp;
        if (!datamodel_->GetValue(key, tmp)) return false;
        if (tmp == "true") {
            value = true;
            return true;
        } else if (tmp == "false") {
            value = false;
            return true;
        } else {
            return false;
        }
    }
    CommitStatus Commit(ITransactionPtr tx) override {
	std::promise<CommitStatus> promise;
	std::future<CommitStatus> future = promise.get_future();
	if (!isLogThread){
		InitializeCommitThread();
	}
        std::lock_guard<std::mutex> lock(mutex);
        transactionPtrQueue_.push_back(std::make_pair(tx, &promise));
        return future;
    }

private:
    IDatamodelPtr datamodel_;
    std::vector<std::pair<ITransactionPtr, std::promise<CommitStatus>*> > transactionPtrQueue_;
    bool stopThread = false;
    bool isCommitThread = false;
    std::mutex mutex;

    void InitializeCommitThread {
	if (isCommitThread){
            return;
	}
	commitThread = std::thread(CommitThreadCycle)
	commitThread.detach()
    }
    void CommitThreadCycle() {
        while(!stopThread) {
            if (!transactionQueue.empty()) {
                std::vector<std::string> localTransactionQueue;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    std::swap(localTransactionQueue, transactionQueue);
                }

                for (auto it = localTransactionQueue.begin(); it != localTransactionQueue.end();
                     it++) {
                    ((*it)[1])->set_value = datamodel_->Commit(((Transaction*)(((*it)[0]).get()))->ops)
                        ? CommitStatus::Success
                        : CommitStatus::Error;

                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
    }
};

std::shared_ptr<ISsypDB> CreateDb(DbSettings settings) {
    return std::make_shared<SsypDB>(settings);
}
