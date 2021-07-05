#include "test_file.h"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE( "Test test", "[factorial]" ) {
    REQUIRE( get_hello() == "Hello world =)" );
}
