project(sdb_lib)

add_library(sdb_lib STATIC)

target_sources(sdb_lib 
PRIVATE
    include/interface.h
    include/settings.h
    src/datamodel.h
    src/storage.h
    src/common.h
    src/datamodel.cpp
)

target_link_libraries(sdb_lib
PRIVATE
    fmt::fmt
)