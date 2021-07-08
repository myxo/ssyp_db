#include "datamodel.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Datamodel", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, DbSettings{});

    Operations ops;
    ops.push_back(Op{"key", "value", Op::Type::Update});
    ops.push_back(Op{"key", "", Op::Type::Remove});
    ops.push_back(Op{"key space test", "value with spaces", Op::Type::Update});

    std::string value;
    datamodel->GetValue("key0", value);
    REQUIRE(value == "no_value");

    datamodel->Commit(ops);
    datamodel->GetValue("key space test", value);
    REQUIRE(value == "value with spaces");

    REQUIRE(storage->GetJournal()[0] == "Update key value");
    REQUIRE(storage->GetJournal()[1] == "Remove key");
}