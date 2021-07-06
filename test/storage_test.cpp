#include "storage.h"

#include <catch2/catch.hpp>

TEST_CASE("Storage", "[write get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    storage->WriteToJournal({ "key,value,update" });
    
    REQUIRE(storage->GetJournal()[0] == "key,value,update");
}

TEST_CASE("TableList", "[]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    storage->AddTable("key,value");
    auto tables = storage->GetTableList();
    
    REQUIRE(tables->TableCount() == 1);
    REQUIRE(tables->GetTable(0) == "key,value");
}