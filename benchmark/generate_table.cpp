#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>

#include "logging.h"
#include "ssyp_db.h"

std::string GenerateValue(double kb_size) {
    const size_t byte_size = kb_size * 1024;
    return std::string(byte_size, 'a');
}

template<typename TimePoint>
double GetElapsedSec(TimePoint const& start, TimePoint const& end) {
    long long elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();
    return static_cast<double>(elapsed_ms) / 1000.0;
}

void ReadValues(std::shared_ptr<ISsypDB> db, int key_num, int iteration) {
    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < iteration; i++) {
        for (int j = 0; j < key_num; j++) {
            std::string key = "key_";
            key.append(std::to_string(i));
            std::string value;
            db->GetValue(key, value);
        }
    }
    auto end = std::chrono::system_clock::now();
    const double elapsed_sec = GetElapsedSec(start, end);
    std::stringstream ss;
    ss << "Reading for " << key_num << " keys " << iteration << " times took " << elapsed_sec << " sec.";
    Info(ss.str());
}

int main(int argc, char* argv[]) {
    SetLogLevel(LogLevel::Debug);
    DbSettings settings;
    settings.journal_limit = 1000;
    settings.filename = "big_bench";

    std::filesystem::remove("big_bench.journal");
    std::filesystem::remove("big_bench.tablelist");

    auto db = CreateDb(settings);

    const int commit_num = 10'000;
    const int value_size_kb = 50;
    std::cout << "Generate DB with " << commit_num
              << " operations. Size of each approx. " << value_size_kb
              << "kb\n";
    std::cout << "Expected useful memory size (approx): "
              << commit_num * value_size_kb / 1024 << "mb\n";

    auto read_thread = std::thread([&]{
        // wait for something in db
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ReadValues(db, commit_num, 1);
    });

    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < commit_num; i++) {
        std::string key = "key_";
        key.append(std::to_string(i));
        std::string value = GenerateValue(value_size_kb);

        auto tx = db->StartTransaction();
        db->SetValue(key, value, tx);
        db->Commit(tx);
    }
    auto end = std::chrono::system_clock::now();
    const double elapsed_sec = GetElapsedSec(start, end);

    std::error_code er;
    size_t journal_size = std::filesystem::file_size("big_bench.journal", er);
    if (er) journal_size = 0;

    size_t table_size = std::filesystem::file_size("big_bench.tablelist", er);
    if (er) table_size = 0;

    std::stringstream ss;
    ss << "Elapsed seconds: " << elapsed_sec << " ("
              << commit_num / elapsed_sec << " commit/sec)\n";
    ss << "Journal size: " << journal_size / 1024 << " kb\n";
    ss << "Tables size: " << table_size / 1024 << " kb\n";
    Info(ss.str());

    read_thread.join();
    db.reset();
    FlushLog();
    return 0;
}