#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "../include/ssyp_db.h"
#include "catch2/catch.hpp"

int main() {
    std::cout << "This is a benchmark template!\n";
    clock_t start, end;
    
    DbSettings settings;
    settings.in_memory = true;
    auto Db = CreateDb(settings);

    auto transaction = Db->StartTransaction();
    Db->SetValue("key1", true, transaction);
    Db->SetValue("key2", 2, transaction);
    Db->SetValue("key3", 3.5, transaction);
    Db->SetValue("key4", "value4", transaction);

    start = clock();
    for (int i = 0; i <= 1000; i++)
    {
        Db->Commit(transaction);
    }
    end = clock();

    printf("1000 commits for %.4f second(s)\n", ((double) end - start) / ((double) CLOCKS_PER_SEC));

    return 0;
}
