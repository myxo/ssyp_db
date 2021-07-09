#include "datamodel.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Datamodel(set get)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, DbSettings{});

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
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, DbSettings{});

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
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, DbSettings{});

    std::string value;

    REQUIRE(datamodel->GetValue("key0", value) == false);
}

TEST_CASE("Datamodel(set get w/ spaces)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, DbSettings{});

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
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, DbSettings{});
    Operations ops;
    ops.push_back(Op{"key space test", "value with spaces", Op::Type::Update});
    datamodel->Commit(ops);
    std::string value;
    REQUIRE(datamodel->GetValue("key space", value) == false);
}