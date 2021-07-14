project(sdb_lib_benchmark)

add_executable(benchmark)

target_sources(benchmark 
PRIVATE
    benchmark.cpp
 )

target_link_libraries (benchmark 
PUBLIC 
    sdb_lib
)

target_include_directories(benchmark
PUBLIC
    ../dependencies
    ../lib/include
)


add_executable(generate_table)

target_sources(generate_table 
PRIVATE
    generate_table.cpp
 )

target_link_libraries (generate_table 
PUBLIC 
    sdb_lib
)

target_include_directories(generate_table
PUBLIC
    ../dependencies
    ../lib/include
)
