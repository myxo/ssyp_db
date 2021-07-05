project(sdb_lib_tests)

add_executable(lib_tests)

target_sources(lib_tests 
PRIVATE
    datamodel_test.cpp
)

target_link_libraries (lib_tests 
PUBLIC 
    sdb_lib
    Catch2::Catch2
)

target_include_directories(lib_tests
PUBLIC
    ../lib/include
    ../lib/src
)