#ifndef SPIRE_STYLE_PARSER_UTILITY_HPP
#define SPIRE_STYLE_PARSER_UTILITY_HPP
#include <boost/optional.hpp>
#include <boost/variant/get.hpp>
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"

namespace Spire {

  /**
   * Returns the token of the specified type from the token list.
   * @param token_parser The token parser.
   * @returns Token iff the current token in the list is of specified type.
   */
  template<typename T>
  boost::optional<T> get_token(const TokenParser& token_parser) {
    if(token_parser.get_size() > 0) {
      auto& value = token_parser.peek().get_value();
      if(value.type() == typeid(T)) {
        return boost::get<T>(value);
      }
    }
    return boost::none;
  }

  /**
   * Checks whether the current token in the token list is a specified one.
   * @param token_parser The token parser.
   * @param token The specifed token.
   * @returns True iff the current token in the list is the specified token.
   */
  template<typename T>
  auto check_token(const TokenParser& token_parser, T token) {
    if(auto current = get_token<T>(token_parser)) {
      return *current == token;
    }
    return false;
  }

  /**
   * Returns the token of the specified type that is adjacent to the previous
   * token from the token list.
   * @param token_parser The token parser.
   * @param previous_token The previous token.
   * @returns Token iff the current token in the token list meets requirements.
   */
  template<typename T>
  boost::optional<T> get_adjacent_token(
      const TokenParser& token_parser, const Token& previous_token) {
    if(token_parser.get_size() > 0) {
      auto& token = token_parser.peek();
      auto& value = token.get_value();
      auto previous_token_length = [&] {
        auto& previous_value = previous_token.get_value();
        if(previous_value.type() == typeid(Identifier)) {
          auto& identifier = boost::get<Identifier>(previous_value);
          return static_cast<int>(identifier.length());
        } else if(previous_value.type() == typeid(Literal)) {
          auto& literal = boost::get<Literal>(previous_value);
          return static_cast<int>(literal.get_value().length());
        }
        return 1;
      }();
      if(value.type() == typeid(T) &&
          token.get_line_number() == previous_token.get_line_number() &&
          token.get_column_number() ==
            previous_token.get_column_number() + previous_token_length) {
        return boost::get<T>(value);
      }
    }
    return boost::none;
  }

  /**
   * Checks whether the current token in the token list is a specified one and
   * is adjacent to the previous token.
   * @param token_parser The token parser.
   * @param previous_token The previous token.
   * @param token The token specified.
   * @returns True iff the next token meets requirements.
   */
  template<typename T>
  auto check_adjacent_token(const TokenParser& token_parser,
      const Token& previous_token, T token) {
    if(auto current = get_adjacent_token<T>(token_parser, previous_token)) {
      return *current == token;
    }
    return false;
  }
}

#endif
