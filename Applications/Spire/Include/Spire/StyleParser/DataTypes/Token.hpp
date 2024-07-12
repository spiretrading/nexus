#ifndef SPIRE_TOKEN_HPP
#define SPIRE_TOKEN_HPP
#include <memory>
#include <string>
#include <boost/variant/variant.hpp>
#include "Spire/StyleParser/DataTypes/DataType.hpp"

namespace Spire {

  /** Represents an identifier. */
  using Identifier = std::string;

  /** Specifies the bracket type. */
  enum class Bracket {

    /** The opening curly bracket. */
    OPEN_CURLY,

    /** The closing curly bracket. */
    CLOSE_CURLY,

    /** The opening square bracket. */
    OPEN_SQUARE,

    /** The closing square bracket. */
    CLOSE_SQUARE,

    /** The opening round bracket. */
    OPEN_ROUND,

    /** The closing round bracket. */
    CLOSE_ROUND
  };

  /** Specifies the predefine keywords. */
  enum class Keyword {
    NONE,
    VISIBILITY,
    VISIBLE,
    INVISIBLE,
    PX,
    PT,
    MS,
    S,
    MIN,
    EQUAL,
    LESS_THAN,
    GREATER_THAN,
    ANY,
    AND,
    OR,
    TILDE,
    EXCLAMATION,
    PERCENTAGE
  };

  /** Specifies the operator types. */
  enum class Operator {

    /** The addition operator. */
    ADDITION,

    /** The subtraction operator. */
    SUBTRACTION,

    /** The multiplication operator. */
    MULTIPLICATION,

    /** The division operator. */
    DIVISION
  };

  /** Specifies the punctuation types. */
  enum class Punctuation {

    /** The colon. */
    COLON,

    /** The comma. */
    COMMA,

    /** The period. */
    PERIOD,

    /** The semi-colon. */
    SEMI_COLON
  };

  /** Represents a literal token. */
  class Literal {
    public:

    /**
     * Constructs a Literal.
     * @param value The literal value.
     * @param type The literal type.
     */
    Literal(std::string value, std::shared_ptr<DataType> type);

    /** Returns the literal value. */
    const std::string& get_value() const;

    /** Returns the literal type. */
    const std::shared_ptr<DataType>& get_type() const;

    private:
    std::string m_value;
    std::shared_ptr<DataType> m_type;
  };

  /** Represents a specific token. */
  class Token {
    public:

      /** Consolidates all token types. */
      using Type = boost::variant<Bracket, Identifier, Keyword, Literal,
        Operator, Punctuation>;

      /**
       * Constructs a Token.
       * @param value The token value.
       * @param line_number The line number.
       * @param column_number The column number.
       */
      Token(Type value, int line_number, int column_number);

      /** Returns the token value. */
      const Type& get_value() const;

      /** Returns the line number of the token. */
      int get_line_number() const;

      /** Returns the column number of the token. */
      int get_column_number() const;

    private:
      Type m_value;
      int m_line_number;
      int m_column_number;
  };
}

#endif
