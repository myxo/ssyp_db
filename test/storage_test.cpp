#include "storage.h"

#include <fstream>

#include "catch2/catch.hpp"

TEST_CASE("InMemoryStorage", "[write get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    storage->WriteToJournal({"key,value"});

    REQUIRE(storage->GetJournal()[0] == "key,value");
}

TEST_CASE("InMemoryTableList", "[push count get merge]") {
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

    storage->MergeTable({0, 1},
                      tables->GetTable(0) + ";" + tables->GetTable(1));
    tables = storage->GetTableList();
    REQUIRE(tables->GetTable(2) == "key1,value1;key2,value2");
}

TEST_CASE("Storage", "[journal]") {
    DbSettings settings;
    std::remove("file.journal");
    settings.filename = "file";
    {
        auto storage = CreateStorage(settings);
        REQUIRE(storage->WriteToJournal({"key1,value1"}));
    }
    {
        auto storage = CreateStorage(settings);
        auto journal = storage->GetJournal();
        REQUIRE(journal.size() == 1);
        REQUIRE(journal[0] == "key1,value1");
        REQUIRE(storage->WriteToJournal({"key2,value2"}));
    }
    {
        auto storage = CreateStorage(settings);
        auto journal = storage->GetJournal();
        REQUIRE(journal.size() == 2);
        REQUIRE(journal[0] == "key1,value1");
        REQUIRE(journal[1] == "key2,value2");
        REQUIRE(storage->WriteToJournal({"key,value"}));
    }
    std::remove("file.journal");
}

TEST_CASE("Journal with zero", "[]") {
    std::remove("file.journal");
    DbSettings settings;
    settings.filename = "file";
    auto storage = CreateStorage(settings);

    std::vector<char> journal_op_vec = {'k', 'e', 'y', '\0', 'v',
                                        'a', 'l', 'u', 'e'};
    std::string journal_op(journal_op_vec.begin(), journal_op_vec.end());

    storage->WriteToJournal({journal_op});
    auto journal = storage->GetJournal();
    REQUIRE(journal[0] == journal_op);
    REQUIRE(journal.size() == 1);
    std::remove("file.journal");
}

TEST_CASE("InFileTableList", "[push count get]") {
    std::remove("file.journal");
    std::remove("file.tablelist");
    DbSettings settings;
    settings.filename = "file";
    {
        auto storage = CreateStorage(settings);
        auto tables = storage->GetTableList();
        storage->WriteToJournal({"key1,value1"});
        storage->PushJournalToTable(storage->GetJournal()[0]);
        REQUIRE(tables->TableCount() == storage->GetTableList()->TableCount());
        REQUIRE(tables->TableCount() == 1);
        REQUIRE(tables->GetTable(0) == "key1,value1");
        REQUIRE(storage->GetJournal().size() == 0);
    }
    {
        auto storage = CreateStorage(settings);
        auto tables = storage->GetTableList();
        storage->WriteToJournal({"key2value2"});
        storage->PushJournalToTable(storage->GetJournal()[0]);
        REQUIRE(tables->TableCount() == 2);
        REQUIRE(tables->GetTable(1) == "key2value2");
    }
    std::remove("file.journal");
    std::remove("file.tablelist");
}
