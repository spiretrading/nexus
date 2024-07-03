#ifndef SPIRE_TOKEN_PARSER_HPP
#define SPIRE_TOKEN_PARSER_HPP
#include <deque>
#include "Spire/StyleParser/DataTypes/Token.hpp"

namespace Spire {

  /** Represents a parser to parse strings into a series of tokens. */
  class TokenParser {
    public:

      /** Constructs a default TokenParser. */
      TokenParser();

      /**
       * Feeds a string to parse.
       * @param input A string to parse.
       */
      void feed(const std::string& input);

      /** Pops the token at the beginning of the list of tokens. */
      Token pop();

      /** Returns the number of tokens. */
      int get_size() const;

    private:
      std::deque<Token> m_tokens;
      int m_line_number;
      int m_column_number;
      std::string m_string_literal;
      bool m_is_in_string;
      int m_string_line_number;
      int m_string_column_number;

      std::string::const_iterator find_string_literal(
        const std::string::const_iterator& begin,
        const std::string::const_iterator& end);
  };
}

#endif
