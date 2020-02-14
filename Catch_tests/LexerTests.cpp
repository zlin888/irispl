#define CATCH_CONFIG_MAIN
#include "catch_lib/catch.hpp"
#include "Lexer.hpp"
#include <iostream>

TEST_CASE("Lexer Test", "[LexerTests]") {  // 12/2/2020 -> 737761
    auto tokens = Lexer::lexer("(define hi)");
    for ( auto &token : tokens) {
        std::cout << token;
    }
    REQUIRE(1 == 1);
}
