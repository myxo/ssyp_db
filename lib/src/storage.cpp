#include "storage.h"
#include "in_memory_storage.h"

IStoragePtr CreateStorage(DbSettings settings) {
    if (settings.in_memory) {
        IStoragePtr storage = std::make_shared<InMemoryStorage>();
        return storage;
    }
}