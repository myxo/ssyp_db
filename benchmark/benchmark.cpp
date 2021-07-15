#include "ssyp_db.h"
#include <chrono>
#include <iostream>

int main() {
    std::cout << "This is a benchmark template!\n";
    clock_t start, end;
    
    DbSettings settings;
    settings.in_memory = true;
    auto db = CreateDb(settings);

    auto transaction = db->StartTransaction();
    db->SetValue("key1", true, transaction);
    db->SetValue("key2", 2, transaction);
    db->SetValue("key3", 3.5, transaction);
    db->SetValue("key4", "value4", transaction);

    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();
    for (int i = 0; i <= 1000; i++)
    {
        db->Commit(transaction);
    }
    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();
    double diff = end - start;
    std::cout << "commits for " << diff << "seconds. " << 1000 / diff << " commits for 1 second.";
    return 0;
}   
