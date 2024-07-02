#include "Spire/StyleParser/DataTypes/Token.hpp"

using namespace Spire;

Bracket::Bracket(BracketType type)
  : m_type(type) {}

Bracket::BracketType Bracket::get_type() const {
  return m_type;
}

Identifier::Identifier(std::string value)
  : m_value(std::move(value)) {}

const std::string& Identifier::get_value() const {
  return m_value;
}

Keyword::Keyword(KeywordType type)
  : m_type(type) {}

Keyword::KeywordType Keyword::get_type() const {
  return m_type;
}

Literal::Literal(std::string value, std::shared_ptr<DataType> type)
  : m_value(std::move(value)),
    m_type(std::move(type)) {}

const std::string& Literal::get_value() const {
  return m_value;
}

const std::shared_ptr<DataType>& Literal::get_type() const {
  return m_type;
}

Operator::Operator(OperatorType type)
  : m_type(type) {}

Operator::OperatorType Operator::get_type() const {
  return m_type;
}

Punctuation::Punctuation(PunctuationType type)
  : m_type(type) {}

Punctuation::PunctuationType Punctuation::get_type() const {
  return m_type;
}

Token::Token(Type value, int line_number, int column_number)
  : m_value(std::move(value)),
    m_line_number(line_number),
    m_column_number(column_number) {}

const Token::Type& Token::get_value() const {
  return m_value;
}

int Token::get_line_number() const {
  return m_line_number;
}

int Token::get_column_number() const {
  return m_column_number;
}
