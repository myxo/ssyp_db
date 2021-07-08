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

    datamodel->Commit(ops);
    std::string value;
    datamodel->GetValue("key", value);

    REQUIRE(value == "value");
    REQUIRE(storage->GetJournal()[0] == "Update key value");
}