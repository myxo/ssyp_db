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
