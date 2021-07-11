#include "datamodel.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Datamodel(set get)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 10;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key", "value", Op::Type::Update});

    std::string value;
    datamodel->Commit(ops);
    datamodel->GetValue("key", value);

    REQUIRE(value == "value");
    REQUIRE(storage->GetJournal()[0] == "Update key value 3");
}

TEST_CASE("Datamodel(set + remove get)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 10;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key", "value", Op::Type::Update});
    ops.push_back(Op{"key", "", Op::Type::Remove});
    ops.push_back(Op{"key", "value1", Op::Type::Update});

    std::string value;
    datamodel->Commit(ops);
    datamodel->GetValue("key", value);

    REQUIRE(value == "value1");
}

TEST_CASE("Datamodel(set get w/ empty key)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 10;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    std::string value;

    REQUIRE(datamodel->GetValue("key0", value) == false);
}

TEST_CASE("Datamodel(set get w/ spaces)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 10;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key with spaces", "value with spaces", Op::Type::Update});

    std::string value;
    datamodel->Commit(ops);
    datamodel->GetValue("key with spaces", value);

    REQUIRE(value == "value with spaces");
}

TEST_CASE("Datamodel(set get w/ spaces and overlapping keys)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 10;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key space test", "value with spaces", Op::Type::Update});
    datamodel->Commit(ops);
    std::string value;

    REQUIRE(datamodel->GetValue("key space", value) == false);
}

TEST_CASE("Datamodel(create table from journal + get value from table)",
          "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 4;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key 1", "value 1", Op::Type::Update});
    ops.push_back(Op{"key 2", "value 2", Op::Type::Update});
    ops.push_back(Op{"key 3", "value 1 2 3", Op::Type::Update});
    ops.push_back(Op{"key 2", "", Op::Type::Remove});
    ops.push_back(Op{"key 1", "value", Op::Type::Update});

    std::string value;

    datamodel->Commit(ops);
    datamodel->Commit({});
    datamodel->GetValue("key 3", value);

    REQUIRE(storage->GetTableList()->GetTable(0) == "");
    REQUIRE(storage->GetTableList()->GetTable(1) ==
            "5 key 15 value5 key 311 value 1 2 3");
    REQUIRE(value == "value 1 2 3");
}