#include <chrono>
#include <filesystem>
#include <iostream>

#include "logging.h"
#include "ssyp_db.h"

std::string GenerateValue(double kb_size) {
    const size_t byte_size = kb_size * 1024;
    return std::string(byte_size, 'a');
}

int main() {
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

    long long elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    const double elapsed_sec = elapsed_ms / 1000.0;

    std::error_code er;
    size_t journal_size = std::filesystem::file_size("big_bench.journal", er);
    if (er) journal_size = 0;

    size_t table_size = std::filesystem::file_size("big_bench.tablelist", er);
    if (er) table_size = 0;

    std::cout << "Elapsed seconds: " << elapsed_sec << " ("
              << commit_num / elapsed_sec << " commit/sec)\n";
    std::cout << "Journal size: " << journal_size / 1024 << " kb\n";
    std::cout << "Tables size: " << table_size / 1024 << " kb\n";

    db.reset();
    FlushLog();
    return 0;
}