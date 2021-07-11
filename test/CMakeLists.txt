project(sdb_lib_tests)

add_executable(lib_tests)

target_sources(lib_tests 
PRIVATE
    datamodel_test.cpp
    storage_test.cpp
    interface_test.cpp
)

target_link_libraries (lib_tests 
PUBLIC 
    sdb_lib
)

target_include_directories(lib_tests
PUBLIC
    ../dependencies
    ../lib/include
    ../lib/src
)

add_test(lib_tests lib_tests)
