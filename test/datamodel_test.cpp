#include "datamodel.h"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Datamodel", "[set get]") {
    auto datamodel = CreateDatamodel(nullptr, DbSettings{});

    Operations ops;
    ops.push_back(Op{ "key", "value", Op::Type::Update });

    datamodel->LSTMCommit(ops);
    std::string value;
    datamodel->LSMTGetValue("key", value);

    REQUIRE(value == "value");
}

TEST_CASE("Datamodel", "[write get]") {
    auto datamodel = CreateDatamodel(nullptr, DbSettings{});

    Operations ops;
    ops.push_back(Op{ "key", "value", Op::Type::Update });

    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);

    datamodel->SerializeOps(ops, storage)

    REQUIRE(storage->GetJournal[0] == "Update key value");
}