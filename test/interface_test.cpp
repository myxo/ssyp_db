#include "../include/interface.h"

#include <string>

#include "catch2/catch.hpp"

TEST_CASE("ISsypDb", "set commit get") {
    DbSettings settings;
    settings.in_memory = false;
    settings.filename = "file";
    auto Db = CreateDb(settings);

    std::string v = "value4";

    auto transaction = Db->StartTransaction();
    Db->SetValue("key1", true, transaction);
    Db->SetValue("key2", 2, transaction);
    Db->SetValue("key3", 3.5, transaction);
    Db->SetValue("key4", v, transaction);
    Db->Commit(transaction);

    bool v1;
    int v2;
    double v3;
    std::string v4;

    REQUIRE(Db->GetValue("key1", &v1));
    REQUIRE(Db->GetValue("key2", &v2));
    REQUIRE(Db->GetValue("key3", &v3));
    REQUIRE(Db->GetValue("key4", v4));

    REQUIRE(v1 == true);
    REQUIRE(v2 == 2);
    REQUIRE(v3 == 3.500000);
    REQUIRE(v4 == "value4");
}
