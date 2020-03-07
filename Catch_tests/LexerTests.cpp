#define CATCH_CONFIG_MAIN
#include "catch_lib/catch.hpp"
#include "Lexer.hpp"
#include <iostream>
#include "Utils.hpp"

using namespace std;

string readFileToOneString(const string&);
vector<Lexer::Token> readExpectedOutput(const string&);

string pathToTestFiles = "/Users/bytedance/CLionProjects/typed-scheme/Catch_tests/test-files/";

TEST_CASE("Lexer Test", "[LexerTests]") {
    string lexerInput = readFileToOneString(pathToTestFiles + "lexer-test-input.txt");
    vector<Lexer::Token> expectedTokens = readExpectedOutput(pathToTestFiles + "lexer-test-output.txt");

    auto tokens = Lexer::tokenize(lexerInput);

    REQUIRE(!tokens.empty());

    for (int i = 0; i < expectedTokens.size(); ++i) {
//        auto expectedToken = expectedTokens[i];
        auto token = tokens[i];
//        cout << expectedTokens[i];
        cout << tokens[i];
//        REQUIRE(expectedToken.string == token.string);
//        REQUIRE(expectedToken.sourceIndex == token.sourceIndex);
    }

}

string readFileToOneString(const string& path) {
    std::fstream fs(path);
    string resultStr;

    if(fs.is_open()){
        std::string line;
        while (std::getline(fs, line)) {
            resultStr += line + '\n';
        }
        fs.close();
    }

    return resultStr;
}

vector<Lexer::Token> readExpectedOutput(const string& path) {
    std::fstream fs(path);
    vector<Lexer::Token> expectedTokens;

    if(fs.is_open()){
        std::string line;
        while (std::getline(fs, line)) {
            string delimiter = ",";
            int splitIndex = line.find(delimiter);

            string first = line.substr(0, splitIndex);
            string second = line.substr(splitIndex + 1, line.size());

            expectedTokens.push_back(Lexer::Token(first, stoi(second, nullptr, 10)));
        }
        fs.close();
    }

    return expectedTokens;
}

TEST_CASE("split test", "[utilsTests]") {
    string string_to_split = "jack/is\\crazy! ?";
    std::regex rgx(R"([\s+/\\\\])");
    std::sregex_token_iterator iter(string_to_split.begin(),
                                    string_to_split.end(),
                                    rgx,
                                    -1);
    std::sregex_token_iterator end;
    for ( ; iter != end; ++iter)
        std::cout << *iter << '\n';
}

