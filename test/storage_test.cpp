#include "storage.h"

#include <catch2/catch.hpp>

TEST_CASE("Storage", "[write get]") {
    auto storage = CreateStorage({ true });
    storage->WriteToJournal({ "key,value,update" });

    REQUIRE(storage->GetJournal()[0] == "key,value,update");
}