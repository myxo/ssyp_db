#include "storage.h"

#include "catch2/catch.hpp"

TEST_CASE("InMemoryStorage", "[write get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    storage->WriteToJournal({"key,value,update"});

    REQUIRE(storage->GetJournal()[0] == "key,value,update");
}

TEST_CASE("InMemoryTableList", "[push count get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);

    auto tables = storage->GetTableList();
    storage->WriteToJournal({"key1,value1"});
    storage->PushJournalToTable(storage->GetJournal()[0]);
    REQUIRE(tables->TableCount() == storage->GetTableList()->TableCount());
    REQUIRE(tables->TableCount() == 1);
    REQUIRE(tables->GetTable(0) == "key1,value1");
    REQUIRE(storage->GetJournal().size() == 0);

    storage->WriteToJournal({"key2,value2"});
    storage->PushJournalToTable(storage->GetJournal()[0]);
    REQUIRE(tables->TableCount() == 2);
    REQUIRE(tables->GetTable(1) == "key2,value2");
}

TEST_CASE("Storage", "[journal]") { 
    DbSettings settings;
    auto storage = CreateStorage(settings);
    REQUIRE(storage->WriteToJournal({"key,value,update"}));

    REQUIRE(storage->GetJournal()[0] == "key,value,update");
}
