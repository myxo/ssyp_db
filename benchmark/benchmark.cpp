#include "ssyp_db.h"
#include <chrono>
#include <iostream>
#include <ctime>
#include <ratio>

int main() {
    std::cout << "This is a benchmark template!\n";
    DbSettings settings;
    settings.in_memory = true;
    auto db = CreateDb(settings);
    auto transaction = db->StartTransaction();
    db->SetValue("key1", true, transaction);
    db->SetValue("key2", 2, transaction);
    db->SetValue("key3", 3.5, transaction);
    db->SetValue("key4", "value4", transaction);
    bool v1;
    int v2;
    double v3;
    std::string v4;

    const int TESTQUANTITY = 5000;

    auto start = std::chrono::high_resolution_clock::now(); // первый тест на количество коммитов за 1 секунду
    for (int i = 1; i <= 1000; i++)
    {
        db->Commit(transaction);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / double(1000);

    std::cout << "1000 commits for " << time << " seconds. " << std::endl;
    std::cout << int(1000 / time) << "commits for 1 second" << std::endl;

    auto start1 = std::chrono::high_resolution_clock::now(); // второй тест на TESTQUANTITY GetValue
    for (size_t i = 1; i <= TESTQUANTITY; i++) {
        db->GetValue("key1", v1);
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    std::cout << TESTQUANTITY << " GetValue() for " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() / double(1000) << " seconds";





    return 0;
}   
