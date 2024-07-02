#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include <boost/algorithm/string.hpp>
#include <regex>
#include <unordered_map>
#include "Spire/StyleParser/DataTypes/FloatType.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/StringType.hpp"

using namespace boost;
using namespace Spire;

namespace {
  enum LITERAL_TYPE {
    STRING,
    HEXADECIMAL,
    INTEGER,
    FLOAT
  };

  const auto BRACKETS = std::unordered_map<std::string, Bracket::BracketType>{
    {"{", Bracket::BracketType::OPEN_CURLY},
    {"}", Bracket::BracketType::CLOSE_CURLY},
    {"[", Bracket::BracketType::OPEN_SQUARE},
    {"]", Bracket::BracketType::CLOSE_SQUARE},
    {"(", Bracket::BracketType::OPEN_ROUND},
    {")", Bracket::BracketType::CLOSE_ROUND}};

  const auto KEYWORDS = std::unordered_map<std::string, Keyword::KeywordType>{
    {"transparent", Keyword::KeywordType::TRANSPARENT},
    {"black", Keyword::KeywordType::BLACK},
    {"white", Keyword::KeywordType::WHITE},
    {"regular", Keyword::KeywordType::REGULAR},
    {"medium", Keyword::KeywordType::MEDIUM},
    {"bold", Keyword::KeywordType::BOLD},
    {"left", Keyword::KeywordType::LEFT},
    {"right", Keyword::KeywordType::RIGHT},
    {"top", Keyword::KeywordType::TOP},
    {"bottom", Keyword::KeywordType::BOTTOM},
    {"center", Keyword::KeywordType::CENTER},
    {"none", Keyword::KeywordType::NONE},
    {"horizontal", Keyword::KeywordType::HORIZONTAL},
    {"vertical", Keyword::KeywordType::VERTICAL},
    {"visible", Keyword::KeywordType::VISIBLE},
    {"invisible", Keyword::KeywordType::INVISIBLE},
    {"px", Keyword::KeywordType::PX},
    {"pt", Keyword::KeywordType::PT},
    {"ms", Keyword::KeywordType::MS},
    {"chain", Keyword::KeywordType::CHAIN},
    {"timeout", Keyword::KeywordType::TIMEOUT},
    {"revert", Keyword::KeywordType::REVERT}};

  const auto OPERATORS =
    std::unordered_map<std::string, Operator::OperatorType>{
      {"+", Operator::OperatorType::ADDITION},
      {"-", Operator::OperatorType::SUBTRACTION},
      {"*", Operator::OperatorType::MULTIPLICATION},
      {"/", Operator::OperatorType::DIVISION}};
  
  const auto PUNCTUATIONS =
    std::unordered_map<std::string, Punctuation::PunctuationType>{
      {":", Punctuation::PunctuationType::COLON},
      {",", Punctuation::PunctuationType::COMMA},
      {".", Punctuation::PunctuationType::PERIOD},
      {";", Punctuation::PunctuationType::SEMI_COLON}};

  const auto LITERAL_REGEX_TEXT_LIST =
    std::unordered_map<LITERAL_TYPE, std::string>{
      {LITERAL_TYPE::STRING, R"(\".*\")"},
      {LITERAL_TYPE::FLOAT, R"([-+]?\d+(\.\d+)([eE][-+]?\d+)?)"},
      {LITERAL_TYPE::HEXADECIMAL,
        R"(0[xX][a-fA-F0-9]{6}\b|\b0[xX][a-fA-F0-9]{8})"},
      {LITERAL_TYPE::INTEGER, R"([-+]?\d+)"}};

  const auto LITERAL_REGEX_TEXT = [] {
    auto text = std::string("(");
    for(auto i = LITERAL_REGEX_TEXT_LIST.begin();
        i != LITERAL_REGEX_TEXT_LIST.end(); ++i) {
      if(i != LITERAL_REGEX_TEXT_LIST.begin()) {
        text += ")|(";
      }
      text += i->second;
    }
    return text + ")";
  }();

  const auto KEYWORD_REGEX_TEXT = [] {
    auto text = std::string();
    for(auto i = KEYWORDS.begin(); i != KEYWORDS.end(); ++i) {
      if(i != KEYWORDS.begin()) {
        text += "|";
      }
      text += i->first;
    }
    return text;
  }();

  const auto ROUND_BRACKET_REGEX_TEXT = std::string(R"([()])");
  const auto BRACKET_REGEX_TEXT = std::string(R"([{}\[\]()])");
  const auto IDENTIFIER_REGEX_TEXT = std::string(R"([a-zA-Z]+\w*)");
  const auto OPERATOR_REGEX_TEXT = std::string(R"([+\-*\/])"); 
  const auto PUNCTUATION_REGEX_TEXT = std::string(R"([:,.;])"); 
  const auto MATH_EXPRESS_ELEMENT_REGEX_TEXT = "(" + ROUND_BRACKET_REGEX_TEXT +
    ")|(" + IDENTIFIER_REGEX_TEXT + ")|(" + LITERAL_REGEX_TEXT +
    ")|(" + OPERATOR_REGEX_TEXT + ")";
  const auto MATH_EXPRESSION_TERMS = "([a-zA-Z]|" +
    LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::FLOAT) + "|" +
    LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::INTEGER) + ")+";
  const auto MATH_EXPRESSION_REGEX_TEXT = MATH_EXPRESSION_TERMS + 
    R"(\s*[+\-*\/]\s*)" + MATH_EXPRESSION_TERMS + R"((\s*[+\-*\/]\s*)" +
    MATH_EXPRESSION_TERMS + ")*"; 
  const auto INTEGER_HEXADECIMAL_TEXT =
    LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::INTEGER) + "|" +
    LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::HEXADECIMAL);
  const auto TOKEN_REGEX_TEXT = "(" + MATH_EXPRESSION_REGEX_TEXT + ")|(" +
    BRACKET_REGEX_TEXT + ")|(" + IDENTIFIER_REGEX_TEXT + ")|(" +
    LITERAL_REGEX_TEXT + ")|(" + KEYWORD_REGEX_TEXT + ")|(" +
    PUNCTUATION_REGEX_TEXT + ")";

  auto get_first_non_whitespace(const std::string::const_iterator& begin,
      const std::string::const_iterator& end) {
    return std::find_if(begin, end, [] (char ch) {
      return !std::isspace(static_cast<unsigned char>(ch));
    });
  }
}

TokenParser::TokenParser()
  : m_line_number(0),
    m_column_number(0) {}

void TokenParser::feed(const std::string& input) {
  auto identifier_regex = std::regex(IDENTIFIER_REGEX_TEXT);
  auto parse_expression = [&] (const std::string& expression, int column) {
    auto round_bracket_regex = std::regex(ROUND_BRACKET_REGEX_TEXT);
    auto math_expression_element_regex =
      std::regex(MATH_EXPRESS_ELEMENT_REGEX_TEXT);
    auto operator_regex = std::regex(OPERATOR_REGEX_TEXT);
    auto start =
      get_first_non_whitespace(expression.cbegin(), expression.cend());
    auto matches = std::smatch();
    while(std::regex_search(start, expression.cend(),
        matches, math_expression_element_regex)) {
      auto matched_text = matches[0].str();
      if(std::regex_search(matched_text, round_bracket_regex)) {
        m_tokens.emplace_back(Bracket(BRACKETS.at(matched_text)),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text, operator_regex) &&
          OPERATORS.contains(matched_text)) {
        m_tokens.emplace_back(Operator(OPERATORS.at(matched_text)),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text,
          std::regex(LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::FLOAT)))) {
        m_tokens.emplace_back(
          Literal(matched_text, std::make_shared<FloatType>()),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text,
          std::regex(LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::INTEGER)))) {
        m_tokens.emplace_back(
          Literal(matched_text, std::make_shared<IntegerType>()),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text, identifier_regex)) {
        m_tokens.emplace_back(Identifier(matched_text),
          m_line_number, m_column_number);
      }
      start =
        get_first_non_whitespace(matches.suffix().first, expression.cend());
      m_column_number = column + std::distance(expression.cbegin(), start);
    }
  };
  auto lines = std::vector<std::string>();
  split(lines, input, is_any_of("\n") );
  auto bracket_regex = std::regex(BRACKET_REGEX_TEXT);
  auto keyword_regex = std::regex(KEYWORD_REGEX_TEXT);
  auto literal_regex = std::regex(LITERAL_REGEX_TEXT);
  auto punctuation_regex = std::regex(PUNCTUATION_REGEX_TEXT);
  auto math_expression_regex = std::regex(MATH_EXPRESSION_REGEX_TEXT);
  auto integer_hexadecimal_regex = std::regex(INTEGER_HEXADECIMAL_TEXT);
  auto token_regex = std::regex(TOKEN_REGEX_TEXT);
  auto matches = std::smatch();
  for(auto& line : lines) {
    auto start = get_first_non_whitespace(line.cbegin(), line.cend());
    m_column_number = std::distance(line.cbegin(), start);
    while(std::regex_search(start, line.cend(), matches, token_regex)) {
      auto matched_text = matches[0].str();
      if(std::regex_search(matched_text, math_expression_regex)) {
        parse_expression(matched_text, m_column_number);
      } else if(std::regex_search(matched_text, bracket_regex) &&
          BRACKETS.contains(matched_text)) {
        m_tokens.emplace_back(Bracket(BRACKETS.at(matched_text)),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text, keyword_regex) &&
          KEYWORDS.contains(matched_text)) {
        m_tokens.emplace_back(Keyword(KEYWORDS.at(matched_text)),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text, literal_regex)) {
        if(std::regex_search(matched_text,
            std::regex(LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::FLOAT)))) {
          m_tokens.emplace_back(
            Literal(matched_text, std::make_shared<FloatType>()),
            m_line_number, m_column_number);
        } else if(std::regex_search(matched_text, integer_hexadecimal_regex)) {
          m_tokens.emplace_back(
            Literal(matched_text, std::make_shared<IntegerType>()),
            m_line_number, m_column_number);
        } else if(std::regex_search(matched_text,
            std::regex(LITERAL_REGEX_TEXT_LIST.at(LITERAL_TYPE::STRING)))) {
          m_tokens.emplace_back(
            Literal(matched_text, std::make_shared<StringType>()),
            m_line_number, m_column_number);
        }
      } else if(std::regex_search(matched_text, punctuation_regex) &&
          PUNCTUATIONS.contains(matched_text)) {
        m_tokens.emplace_back(Punctuation(PUNCTUATIONS.at(matched_text)),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text, identifier_regex)) {
        m_tokens.emplace_back(Identifier(matched_text),
          m_line_number, m_column_number);
      }
      start = get_first_non_whitespace(matches.suffix().first, line.cend());
      m_column_number = std::distance(line.cbegin(), start);
    }
    ++m_line_number;
  }
}

Token TokenParser::pop() {
  auto token = m_tokens.front();
  m_tokens.pop_front();
  return token;
}

int TokenParser::get_size() const {
  return static_cast<int>(m_tokens.size());
}
