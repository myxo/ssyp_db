#include "storage.h"

#include <catch2/catch.hpp>

TEST_CASE("Storage", "[write get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    storage->WriteToJournal({ "key,value,update" });
    
    REQUIRE(storage->GetJournal()[0] == "key,value,update");
}

TEST_CASE("TableList", "[count get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    auto tables = storage->GetTableList();
    REQUIRE(tables->TableCount() == 0);

    storage->AddTable("key1,value1");
    REQUIRE(tables->TableCount() == storage->GetTableList()->TableCount());
    REQUIRE(tables->TableCount() == 1);
    REQUIRE(tables->GetTable(0) == "key1,value1");

    storage->AddTable("key2,value2");
    REQUIRE(tables->TableCount() == 2);
    REQUIRE(tables->GetTable(1) == "key2,value2");
}
