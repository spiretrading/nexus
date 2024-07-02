#include <doctest/doctest.h>
#include "Spire/StyleParser/DataTypes/FloatType.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/StringType.hpp"
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include "Spire/Spire/ListModel.hpp"

using namespace Spire;

namespace {
  template<typename T>
  struct Result {
    T m_value;
    int m_line_number;
    int m_column_number;
  };

  struct ParseResults {
    std::vector<Result<Bracket>> m_brackets;
    std::vector<Result<Identifier>> m_identifiers;
    std::vector<Result<Keyword>> m_keywords;
    std::vector<Result<Operator>> m_operators;
    std::vector<Result<Literal>> m_literals;
    std::vector<Result<Punctuation>> m_punctuations;
  };

  void parse(TokenParser& parser, ParseResults& results) {
    while(parser.get_size() != 0) {
      auto token = parser.pop();
      visit(token.get_value(),
        [&] (const Bracket& bracket) {
          results.m_brackets.push_back(
            {bracket, token.get_line_number(), token.get_column_number()});
        },
        [&] (const Identifier& identifier) {
          results.m_identifiers.push_back(
            {identifier, token.get_line_number(), token.get_column_number()});
        },
        [&] (const Keyword& keyword) {
          results.m_keywords.push_back(
            {keyword, token.get_line_number(), token.get_column_number()});
        },
        [&] (const Operator& op) {
          results.m_operators.push_back(
            {op, token.get_line_number(), token.get_column_number()});
        },
        [&] (const Literal& literal) {
          results.m_literals.push_back(
            {literal, token.get_line_number(), token.get_column_number()});
        },
        [&] (const Punctuation& punctuation) {
          results.m_punctuations.push_back(
            {punctuation, token.get_line_number(), token.get_column_number()});
        });
    }
  }
}

TEST_SUITE("TokenParser") {
  TEST_CASE("basic") {
    auto parser = TokenParser();
    parser.feed(R"(
      { max_value: -1 * m - f; 10.2 (medium, bold) ["Test", b / -1.5]}
    )");
    REQUIRE(parser.get_size() == 23);
    auto results = ParseResults();
    parse(parser, results);
    REQUIRE(results.m_brackets.size() == 6);
    REQUIRE(results.m_brackets[0].m_value.get_type() ==
      Bracket::BracketType::OPEN_CURLY);
    REQUIRE(results.m_brackets[0].m_column_number == 6);
    REQUIRE(results.m_brackets[1].m_value.get_type() ==
      Bracket::BracketType::OPEN_ROUND);
    REQUIRE(results.m_brackets[1].m_column_number == 36);
    REQUIRE(results.m_brackets[2].m_value.get_type() ==
      Bracket::BracketType::CLOSE_ROUND);
    REQUIRE(results.m_brackets[2].m_column_number == 49);
    REQUIRE(results.m_brackets[3].m_value.get_type() ==
      Bracket::BracketType::OPEN_SQUARE);
    REQUIRE(results.m_brackets[3].m_column_number == 51);
    REQUIRE(results.m_brackets[4].m_value.get_type() ==
      Bracket::BracketType::CLOSE_SQUARE);
    REQUIRE(results.m_brackets[4].m_column_number == 68);
    REQUIRE(results.m_brackets[5].m_value.get_type() ==
      Bracket::BracketType::CLOSE_CURLY);
    REQUIRE(results.m_brackets[5].m_column_number == 69);
    REQUIRE(results.m_identifiers.size() == 4);
    REQUIRE(results.m_identifiers[0].m_value.get_value() == "max_value");
    REQUIRE(results.m_identifiers[0].m_column_number == 8);
    REQUIRE(results.m_identifiers[1].m_value.get_value() == "m");
    REQUIRE(results.m_identifiers[1].m_column_number == 24);
    REQUIRE(results.m_identifiers[2].m_value.get_value() == "f");
    REQUIRE(results.m_identifiers[2].m_column_number == 28);
    REQUIRE(results.m_identifiers[3].m_value.get_value() == "b");
    REQUIRE(results.m_identifiers[3].m_column_number == 60);
    REQUIRE(results.m_keywords.size() == 2);
    REQUIRE(results.m_keywords[0].m_value.get_type() ==
      Keyword::KeywordType::MEDIUM);
    REQUIRE(results.m_keywords[0].m_column_number == 37);
    REQUIRE(results.m_keywords[1].m_value.get_type() ==
      Keyword::KeywordType::BOLD);
    REQUIRE(results.m_keywords[1].m_column_number == 45);
    REQUIRE(results.m_literals.size() == 4);
    REQUIRE(*results.m_literals[0].m_value.get_type() == IntegerType());
    REQUIRE(results.m_literals[0].m_column_number == 19);
    REQUIRE(*results.m_literals[1].m_value.get_type() == FloatType());
    REQUIRE(results.m_literals[1].m_column_number == 31);
    REQUIRE(*results.m_literals[2].m_value.get_type() == StringType());
    REQUIRE(results.m_literals[2].m_column_number == 52);
    REQUIRE(*results.m_literals[3].m_value.get_type() == FloatType());
    REQUIRE(results.m_literals[3].m_column_number == 64);
    REQUIRE(results.m_punctuations.size() == 4);
    REQUIRE(results.m_punctuations[0].m_value.get_type() ==
      Punctuation::PunctuationType::COLON);
    REQUIRE(results.m_punctuations[0].m_column_number == 17);
    REQUIRE(results.m_punctuations[1].m_value.get_type() ==
      Punctuation::PunctuationType::SEMI_COLON);
    REQUIRE(results.m_punctuations[1].m_column_number == 29);
    REQUIRE(results.m_punctuations[2].m_value.get_type() ==
      Punctuation::PunctuationType::COMMA);
    REQUIRE(results.m_punctuations[2].m_column_number == 43);
    REQUIRE(results.m_punctuations[3].m_value.get_type() ==
      Punctuation::PunctuationType::COMMA);
    REQUIRE(results.m_punctuations[3].m_column_number == 58);
    REQUIRE(results.m_operators.size() == 3);
    REQUIRE(results.m_operators[0].m_value.get_type() ==
      Operator::OperatorType::MULTIPLICATION);
    REQUIRE(results.m_operators[0].m_column_number == 22);
    REQUIRE(results.m_operators[1].m_value.get_type() ==
      Operator::OperatorType::SUBTRACTION);
    REQUIRE(results.m_operators[1].m_column_number == 26);
    REQUIRE(results.m_operators[2].m_value.get_type() ==
      Operator::OperatorType::DIVISION);
    REQUIRE(results.m_operators[2].m_column_number == 62);
  }

  TEST_CASE("parse_single_line") {
    auto parser = TokenParser();
    parser.feed("Box { border_size: 1px; }");
    REQUIRE(parser.get_size() == 8);
    auto results = ParseResults();
    parse(parser, results);
    REQUIRE(results.m_brackets.size() == 2);
    REQUIRE(results.m_brackets[0].m_value.get_type() ==
      Bracket::BracketType::OPEN_CURLY);
    REQUIRE(results.m_brackets[0].m_line_number == 0);
    REQUIRE(results.m_brackets[0].m_column_number == 4);
    REQUIRE(results.m_brackets[1].m_value.get_type() ==
      Bracket::BracketType::CLOSE_CURLY);
    REQUIRE(results.m_brackets[1].m_line_number == 0);
    REQUIRE(results.m_brackets[1].m_column_number == 24);
    REQUIRE(results.m_identifiers.size() == 2);
    REQUIRE(results.m_identifiers[0].m_value.get_value() == "Box");
    REQUIRE(results.m_identifiers[0].m_line_number == 0);
    REQUIRE(results.m_identifiers[0].m_column_number == 0);
    REQUIRE(results.m_identifiers[1].m_value.get_value() == "border_size");
    REQUIRE(results.m_identifiers[1].m_line_number == 0);
    REQUIRE(results.m_identifiers[1].m_column_number == 6);
    REQUIRE(results.m_keywords.size() == 1);
    REQUIRE(results.m_keywords[0].m_value.get_type() ==
      Keyword::KeywordType::PX);
    REQUIRE(results.m_keywords[0].m_line_number == 0);
    REQUIRE(results.m_keywords[0].m_column_number == 20);
    REQUIRE(results.m_literals.size() == 1);
    REQUIRE(*results.m_literals[0].m_value.get_type() == IntegerType());
    REQUIRE(results.m_literals[0].m_line_number == 0);
    REQUIRE(results.m_literals[0].m_column_number == 19);
    REQUIRE(results.m_punctuations.size() == 2);
    REQUIRE(results.m_punctuations[0].m_value.get_type() ==
      Punctuation::PunctuationType::COLON);
    REQUIRE(results.m_punctuations[0].m_line_number == 0);
    REQUIRE(results.m_punctuations[0].m_column_number == 17);
    REQUIRE(results.m_punctuations[1].m_value.get_type() ==
      Punctuation::PunctuationType::SEMI_COLON);
    REQUIRE(results.m_punctuations[1].m_line_number == 0);
    REQUIRE(results.m_punctuations[1].m_column_number == 22);
    REQUIRE(results.m_operators.size() == 0);
  }

  TEST_CASE("parse_multiple_lines") {
    auto parser = TokenParser();
    parser.feed(R"(
      TextBox {
        text_style: regular 12px "Roboto" black;
        border_color: chain(timeout(0xB71C1C, 55.5ms), revert);
      }
    )");
    REQUIRE(parser.get_size() == 26);
    auto results = ParseResults();
    parse(parser, results);
    REQUIRE(results.m_brackets.size() == 6);
    REQUIRE(results.m_brackets[0].m_value.get_type() ==
      Bracket::BracketType::OPEN_CURLY);
    REQUIRE(results.m_brackets[0].m_line_number == 1);
    REQUIRE(results.m_brackets[0].m_column_number == 14);
    REQUIRE(results.m_brackets[1].m_value.get_type() ==
      Bracket::BracketType::OPEN_ROUND);
    REQUIRE(results.m_brackets[1].m_line_number == 3);
    REQUIRE(results.m_brackets[1].m_column_number == 27);
    REQUIRE(results.m_brackets[2].m_value.get_type() ==
      Bracket::BracketType::OPEN_ROUND);
    REQUIRE(results.m_brackets[2].m_line_number == 3);
    REQUIRE(results.m_brackets[2].m_column_number == 35);
    REQUIRE(results.m_brackets[3].m_value.get_type() ==
      Bracket::BracketType::CLOSE_ROUND);
    REQUIRE(results.m_brackets[3].m_line_number == 3);
    REQUIRE(results.m_brackets[3].m_column_number == 52);
    REQUIRE(results.m_brackets[4].m_value.get_type() ==
      Bracket::BracketType::CLOSE_ROUND);
    REQUIRE(results.m_brackets[4].m_line_number == 3);
    REQUIRE(results.m_brackets[4].m_column_number == 61);
    REQUIRE(results.m_brackets[5].m_value.get_type() ==
      Bracket::BracketType::CLOSE_CURLY);
    REQUIRE(results.m_brackets[5].m_line_number == 4);
    REQUIRE(results.m_brackets[5].m_column_number == 6);
    REQUIRE(results.m_identifiers.size() == 3);
    REQUIRE(results.m_identifiers[0].m_value.get_value() == "TextBox");
    REQUIRE(results.m_identifiers[0].m_line_number == 1);
    REQUIRE(results.m_identifiers[0].m_column_number == 6);
    REQUIRE(results.m_identifiers[1].m_value.get_value() == "text_style");
    REQUIRE(results.m_identifiers[1].m_line_number == 2);
    REQUIRE(results.m_identifiers[1].m_column_number == 8);
    REQUIRE(results.m_identifiers[2].m_value.get_value() == "border_color");
    REQUIRE(results.m_identifiers[2].m_line_number == 3);
    REQUIRE(results.m_identifiers[2].m_column_number == 8);
    REQUIRE(results.m_keywords.size() == 7);
    REQUIRE(results.m_keywords[0].m_value.get_type() ==
      Keyword::KeywordType::REGULAR);
    REQUIRE(results.m_keywords[0].m_line_number == 2);
    REQUIRE(results.m_keywords[0].m_column_number == 20);
    REQUIRE(results.m_keywords[1].m_value.get_type() ==
      Keyword::KeywordType::PX);
    REQUIRE(results.m_keywords[1].m_line_number == 2);
    REQUIRE(results.m_keywords[1].m_column_number == 30);
    REQUIRE(results.m_keywords[2].m_value.get_type() ==
      Keyword::KeywordType::BLACK);
    REQUIRE(results.m_keywords[2].m_line_number == 2);
    REQUIRE(results.m_keywords[2].m_column_number == 42);
    REQUIRE(results.m_keywords[3].m_value.get_type() ==
      Keyword::KeywordType::CHAIN);
    REQUIRE(results.m_keywords[3].m_line_number == 3);
    REQUIRE(results.m_keywords[3].m_column_number == 22);
    REQUIRE(results.m_literals.size() == 4);
    REQUIRE(*results.m_literals[0].m_value.get_type() == IntegerType());
    REQUIRE(results.m_literals[0].m_line_number == 2);
    REQUIRE(results.m_literals[0].m_column_number == 28);
    REQUIRE(*results.m_literals[1].m_value.get_type() == StringType());
    REQUIRE(results.m_literals[1].m_line_number == 2);
    REQUIRE(results.m_literals[1].m_column_number == 33);
    REQUIRE(*results.m_literals[2].m_value.get_type() == IntegerType());
    REQUIRE(results.m_literals[2].m_line_number == 3);
    REQUIRE(results.m_literals[2].m_column_number == 36);
    REQUIRE(*results.m_literals[3].m_value.get_type() == FloatType());
    REQUIRE(results.m_literals[3].m_line_number == 3);
    REQUIRE(results.m_literals[3].m_column_number == 46);
    REQUIRE(results.m_punctuations.size() == 6);
    REQUIRE(results.m_punctuations[0].m_value.get_type() ==
      Punctuation::PunctuationType::COLON);
    REQUIRE(results.m_punctuations[0].m_line_number == 2);
    REQUIRE(results.m_punctuations[0].m_column_number == 18);
    REQUIRE(results.m_punctuations[1].m_value.get_type() ==
      Punctuation::PunctuationType::SEMI_COLON);
    REQUIRE(results.m_punctuations[1].m_line_number == 2);
    REQUIRE(results.m_punctuations[1].m_column_number == 47);
    REQUIRE(results.m_punctuations[2].m_value.get_type() ==
      Punctuation::PunctuationType::COLON);
    REQUIRE(results.m_punctuations[2].m_line_number == 3);
    REQUIRE(results.m_punctuations[2].m_column_number == 20);
    REQUIRE(results.m_punctuations[3].m_value.get_type() ==
      Punctuation::PunctuationType::COMMA);
    REQUIRE(results.m_punctuations[3].m_line_number == 3);
    REQUIRE(results.m_punctuations[3].m_column_number == 44);
    REQUIRE(results.m_punctuations[4].m_value.get_type() ==
      Punctuation::PunctuationType::COMMA);
    REQUIRE(results.m_punctuations[4].m_line_number == 3);
    REQUIRE(results.m_punctuations[4].m_column_number == 53);
    REQUIRE(results.m_operators.size() == 0);
  }
}
