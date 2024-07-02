#ifndef SPIRE_TOKEN_HPP
#define SPIRE_TOKEN_HPP
#include <boost/variant/variant.hpp>
#include <memory>
#include <string>
#include "Spire/StyleParser/DataTypes/DataType.hpp"

namespace Spire {

  /** Represents a bracket token. */
  class Bracket {
    public:

      /** Lists the bracket types. */
      enum class BracketType {

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

      /**
       * Constructs a Bracket.
       * @param type The bracket type.
       */
      explicit Bracket(BracketType type);

      /** Returns the bracket type. */
      BracketType get_type() const;

    private:
      BracketType m_type;
  };

  /** Represents an identifier token. */
  class Identifier {
    public:

      /**
       * Constructs an Identifier.
       * @param value The identifier value.
       */
      explicit Identifier(std::string value);

      /** Returns the identifier value. */
      const std::string& get_value() const;

    private:
      std::string m_value;
  };

  /** Represents a keyword token. */
  class Keyword {
    public:

      /** Lists the predefine keywords. */
      enum class KeywordType {
        TRANSPARENT,
        BLACK,
        WHITE,
        REGULAR,
        MEDIUM,
        BOLD,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        CENTER,
        NONE,
        HORIZONTAL,
        VERTICAL,
        VISIBLE,
        INVISIBLE,
        PX,
        PT,
        MS,
        CHAIN,
        TIMEOUT,
        REVERT
      };

      /**
       * Constructs a Keyword.
       * @param type The keyword type.
       */
      explicit Keyword(KeywordType type);

      /** Returns the keyword type. */
      KeywordType get_type() const;

    private:
      KeywordType m_type;
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

  /** Represents an operator token. */
  class Operator {
    public:

      /** Lists the operator types. */
      enum class OperatorType {

        /** The addition operator. */
        ADDITION,

        /** The subtraction operator. */
        SUBTRACTION,

        /** The multiplication operator. */
        MULTIPLICATION,

        /** The division operator. */
        DIVISION
      };

      /**
       * Constructs an Operator.
       * @param type The operator type.
       */
      explicit Operator(OperatorType type);

      /** Returns the operator type. */
      OperatorType get_type() const;

    private:
      OperatorType m_type;
  };

  /** Represents a punctuation token. */
  class Punctuation {
    public:

      /** Lists the punctuation types. */
      enum class PunctuationType {

        /** The colon. */
        COLON,

        /** The comma. */
        COMMA,

        /** The period. */
        PERIOD,

        /** The semi-colon. */
        SEMI_COLON
      };

      /**
       * Constructs a Punctuation.
       * @param type The punctuation type.
       */
      explicit Punctuation(PunctuationType type);

      /** Returns the punctuation type. */
      PunctuationType get_type() const;

    private:
      PunctuationType m_type;
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
