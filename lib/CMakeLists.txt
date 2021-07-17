project(sdb_lib)

add_library(sdb_lib STATIC)

target_sources(sdb_lib 
PRIVATE
    include/ssyp_db.h
    include/settings.h
    include/logging.h
    src/datamodel.h
    src/storage.h
    src/in_memory_storage.h
    src/datamodel.cpp
    src/storage.cpp
    src/in_memory_storage.cpp
    src/logging.cpp
    src/ssyp_db.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(sdb_lib Threads::Threads)

target_link_libraries(sdb_lib
PRIVATE
)

target_include_directories(sdb_lib
PUBLIC
    ../dependencies
    include
)
