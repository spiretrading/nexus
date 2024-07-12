#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include <regex>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include "Spire/StyleParser/DataTypes/FloatType.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/StringType.hpp"

using namespace boost;
using namespace Spire;

namespace {
  enum LITERAL_TYPE {
    HEXADECIMAL,
    INTEGER,
    FLOAT
  };

  const auto BRACKETS = std::unordered_map<std::string, Bracket>{
    {"{", Bracket::OPEN_CURLY},
    {"}", Bracket::CLOSE_CURLY},
    {"[", Bracket::OPEN_SQUARE},
    {"]", Bracket::CLOSE_SQUARE},
    {"(", Bracket::OPEN_ROUND},
    {")", Bracket::CLOSE_ROUND}};

  const auto KEYWORDS = std::unordered_map<std::string, Keyword>{
    //{"transparent", Keyword::TRANSPARENT},
    //{"black", Keyword::BLACK},
    //{"white", Keyword::WHITE},
    //{"regular", Keyword::REGULAR},
    //{"medium", Keyword::MEDIUM},
    //{"bold", Keyword::BOLD},
    //{"left", Keyword::LEFT},
    //{"right", Keyword::RIGHT},
    //{"top", Keyword::TOP},
    //{"bottom", Keyword::BOTTOM},
    //{"center", Keyword::CENTER},
    {"none", Keyword::NONE},
    //{"horizontal", Keyword::HORIZONTAL},
    //{"vertical", Keyword::VERTICAL},
    {"visibility", Keyword::VISIBILITY},
    {"visible", Keyword::VISIBLE},
    {"invisible", Keyword::INVISIBLE},
    {"px", Keyword::PX},
    {"pt", Keyword::PT},
    {"ms", Keyword::MS},
    {"s", Keyword::S},
    //{"chain", Keyword::CHAIN},
    //{"timeout", Keyword::TIMEOUT},
    //{"revert", Keyword::REVERT},
    {"=", Keyword::EQUAL},
    {"<", Keyword::LESS_THAN},
    {">", Keyword::GREATER_THAN},
    {"any", Keyword::ANY},
    {"and", Keyword::AND},
    {"or", Keyword::OR},
    {"~", Keyword::TILDE},
    {"!", Keyword::EXCLAMATION},
    {"%", Keyword::PERCENTAGE}};

  const auto OPERATORS =
    std::unordered_map<std::string, Operator>{
      {"+", Operator::ADDITION},
      {"-", Operator::SUBTRACTION},
      {"*", Operator::MULTIPLICATION},
      {"/", Operator::DIVISION}};
  
  const auto PUNCTUATIONS =
    std::unordered_map<std::string, Punctuation>{
      {":", Punctuation::COLON},
      {",", Punctuation::COMMA},
      {".", Punctuation::PERIOD},
      {";", Punctuation::SEMI_COLON}};

  const auto LITERAL_REGEX_TEXT_LIST =
    std::unordered_map<LITERAL_TYPE, std::string>{
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
    m_column_number(0),
    m_is_in_string(false),
    m_string_line_number(0),
    m_string_column_number(0) {}

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
  auto parse_string_literal = [&] (const std::string::const_iterator& begin,
      const std::string& text) {
    auto start = get_first_non_whitespace(begin, text.cend());
    m_column_number = std::distance(text.cbegin(), start);
    if(auto new_start = find_string_literal(start, text.cend());
        new_start != start) {
      start = get_first_non_whitespace(new_start, text.cend());
      m_column_number = std::distance(text.cbegin(), start);
    }
    return start;
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
    auto start = parse_string_literal(line.cbegin(), line);
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
        to_lower(matched_text);
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
        }
      } else if(std::regex_search(matched_text, punctuation_regex) &&
          PUNCTUATIONS.contains(matched_text)) {
        m_tokens.emplace_back(Punctuation(PUNCTUATIONS.at(matched_text)),
          m_line_number, m_column_number);
      } else if(std::regex_search(matched_text, identifier_regex)) {
        m_tokens.emplace_back(Identifier(matched_text),
          m_line_number, m_column_number);
      }
      start = parse_string_literal(matches.suffix().first, line);
    }
    ++m_line_number;
  }
}

const Token& Spire::TokenParser::peek() const {
  if(m_tokens.empty()) {
    throw std::runtime_error("No tokens.");
  }
  return m_tokens.front();
}

void TokenParser::pop() {
  if(m_tokens.empty()) {
    throw std::runtime_error("No tokens.");
  }
  m_tokens.pop_front();
}

int TokenParser::get_size() const {
  return static_cast<int>(m_tokens.size());
}

std::string::const_iterator TokenParser::find_string_literal(
    const std::string::const_iterator& begin,
    const std::string::const_iterator& end) {
  if(!m_is_in_string && (begin == end || *begin != '"')) {
    return begin;
  }
  auto i = begin;
  for(; i != end; ++i) {
    if(*i == '"' && (i == begin || *(i - 1) != '\\')) {
      if(m_is_in_string) {
        m_tokens.emplace_back(
          Literal(m_string_literal + "\"", std::make_shared<StringType>()),
          m_string_line_number, m_string_column_number);
        m_string_literal.clear();
        m_is_in_string = !m_is_in_string;
        return i + 1;
      }
      m_string_literal = '"';
      m_string_line_number = m_line_number;
      m_string_column_number = m_column_number;
      m_is_in_string = !m_is_in_string;
    } else if(m_is_in_string) {
      m_string_literal += *i;
    }
  }
  return i;
}
