#include "datamodel.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Datamodel(set get)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

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
    auto datamodel = CreateDatamodel(storage, settings);

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
    auto datamodel = CreateDatamodel(storage, settings);

    std::string value;

    REQUIRE(datamodel->GetValue("key0", value) == false);
}

TEST_CASE("Datamodel(set get w/ spaces)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

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
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key space test", "value with spaces", Op::Type::Update});
    datamodel->Commit(ops);
    std::string value;

    REQUIRE(datamodel->GetValue("key space", value) == false);
}

TEST_CASE("Datamodel(create table from journal + get value from table)",
          "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 4;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key 1", "value 1", Op::Type::Update});
    ops.push_back(Op{"key 2", "value 2", Op::Type::Update});
    ops.push_back(Op{"key 3", "value 1 2 3", Op::Type::Update});
    ops.push_back(Op{"key 2", "", Op::Type::Remove});
    ops.push_back(Op{"key 1", "value", Op::Type::Update});

    std::string value;

    datamodel->Commit(ops);
    datamodel->Commit({});
    datamodel->GetValue("key 1", value);

    REQUIRE(storage->GetTableList()->GetTable(0) == " 1");
    REQUIRE(storage->GetTableList()->GetTable(1) ==
            "5 key 15 value5 key 20 5 key 311 value 1 2 3 1");
    REQUIRE(value == "value");
}

TEST_CASE("Datamodel(empty values in table)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 1;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key", "value", Op::Type::Update});
    ops.push_back(Op{"key", "", Op::Type::Remove});

    std::string value;

    datamodel->Commit(ops);
    datamodel->Commit({});

    REQUIRE(storage->GetTableList()->GetTable(1) == "3 key0  1");
    REQUIRE(datamodel->GetValue("key", value) == false);
}

 TEST_CASE("Datamodel(merge tables)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 1;
    settings.table_limit = 3;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops1;
    ops1.push_back(Op{"key 1", "value1", Op::Type::Update});
    ops1.push_back(Op{"key2", "value2", Op::Type::Update});

    std::string value;

    datamodel->Commit(ops1);
    datamodel->Commit({});

    Operations ops2;
    ops2.push_back(Op{"key3", "value3", Op::Type::Update});
    ops2.push_back(Op{"key 1", "value", Op::Type::Update});

    datamodel->Commit(ops2);
    datamodel->Commit({});

    REQUIRE(storage->GetTableList()->GetTable(0) ==
            "5 key 15 value4 key26 value24 key36 value3 2");
}

TEST_CASE("Datamodel(merge many tables)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 1;
    settings.table_limit = 3;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key1", "value1", Op::Type::Update});
    ops.push_back(Op{"key2", "value2", Op::Type::Update});

    for (int i = 0; i < 21; i++) {
        datamodel->Commit(ops);
    }

    REQUIRE(storage->GetTableList()->GetTable(0) ==
            "4 key16 value14 key26 value2 3");
    REQUIRE(storage->GetTableList()->GetTable(1) ==
            "4 key16 value14 key26 value2 2");
    REQUIRE(storage->GetTableList()->GetTable(2) ==
            "4 key16 value14 key26 value2 1");
}

TEST_CASE("Datamodel(many tables on the last level)", "[set get]") {
    DbSettings settings;
    settings.in_memory = true;
    settings.journal_limit = 1;
    settings.table_limit = 2;
    auto storage = CreateStorage(settings);
    auto datamodel = CreateDatamodel(storage, settings);

    Operations ops;
    ops.push_back(Op{"key1", "value1", Op::Type::Update});
    ops.push_back(Op{"key2", "value2", Op::Type::Update});

    for (int i = 0; i < 9; i++) {
        datamodel->Commit(ops);
    }

    REQUIRE(storage->GetTableList()->GetTable(0) ==
            "4 key16 value14 key26 value2 2");
    REQUIRE(storage->GetTableList()->GetTable(1) ==
            "4 key16 value14 key26 value2 2");
    REQUIRE(storage->GetTableList()->GetTable(2) ==
            "4 key16 value14 key26 value2 2");
}