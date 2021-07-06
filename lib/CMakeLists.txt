project(sdb_lib)

add_library(sdb_lib STATIC)

target_sources(sdb_lib 
PRIVATE
    include/interface.h
    include/settings.h
    src/datamodel.h
    src/storage.h
    src/in_memory_storage.h
    src/datamodel.cpp
    src/storage.cpp
    src/in_memory_storage.cpp
 )

target_link_libraries(sdb_lib
PRIVATE
)

target_include_directories(sdb_lib
PUBLIC
    ../dependencies
)