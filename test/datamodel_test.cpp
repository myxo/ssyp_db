#include "datamodel.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Datamodel", "[set get]") {
    auto datamodel = CreateDatamodel(nullptr, DbSettings{});

    Operations ops;
    ops.push_back(Op{"key", "value", Op::Type::Update});

    datamodel->Commit(ops);
    std::string value;
    datamodel->GetValue("key", value);

    REQUIRE(value == "value");
}


