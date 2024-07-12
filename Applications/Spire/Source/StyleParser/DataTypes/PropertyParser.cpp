#include "Spire/StyleParser/DataTypes/PropertyParser.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/Utility.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace Spire;
using namespace Styles;

namespace {
  class PropertyRegistry {
    public:
      static std::shared_ptr<PropertyRegistry> get_instance() {
        static auto instance = std::make_shared<PropertyRegistry>();
        return instance;
      }

      void register_converter(const std::string& name,
          PropertyConverter converter) {
        m_converters[to_lower_copy(name)] = converter;
      }

      void register_function_converter(const std::string& name,
          FunctionConverter converter) {
        m_function_converters[to_lower_copy(name)] = converter;
      }

      PropertyConverter get_converter(const std::string& name) const {
        if(auto i = m_converters.find(name); i != m_converters.end()) {
          return i->second;
        }
        return {};
      }

      FunctionConverter get_function_converter(const std::string& name) const {
        if(auto i = m_function_converters.find(name);
            i != m_function_converters.end()) {
          return i->second;
        }
        return {};
      }

    private:
      std::unordered_map<std::string, PropertyConverter> m_converters;
      std::unordered_map<std::string, FunctionConverter> m_function_converters;
  };

  auto check_length_property(TokenParser& token_parser,
      const Token& previous_token) {
    if(previous_token.get_value().type() == typeid(Literal)) {
      if(check_token(token_parser, Keyword::PX) ||
          check_token(token_parser, Keyword::PT)) {
        if(get_adjacent_token<Keyword>(token_parser, previous_token)) {
          return true;
        } else {
          throw std::runtime_error("No space between length and unit.");
        }
      }
    }
    return false;
  }

  auto check_time_property(TokenParser& token_parser,
      const Token& previous_token) {
    if(previous_token.get_value().type() == typeid(Literal)) {
      if(check_token(token_parser, Keyword::MS) ||
          check_token(token_parser, Keyword::S) ||
          check_token(token_parser, Keyword::MIN)) {
        if(get_adjacent_token<Keyword>(token_parser, previous_token)) {
          return true;
        } else {
          throw std::runtime_error("No space between time and unit.");
        }
      }
    }
    return false;
  }

  void insert(Rule& rule, const std::vector<Property>& properties) {
    for(auto& property : properties) {
      rule.set(property);
    }
  }

  optional<Property> parse_function(TokenParser& token_parser,
      const Token& token) {
    auto& value = token.get_value();
    if(value.type() == typeid(Identifier) && check_adjacent_token(
        token_parser, token, Bracket::OPEN_ROUND)) {
      auto name = boost::get<Identifier>(value);
      auto values = std::vector<PropertyValue>();
      token_parser.pop();
      while(!check_token(token_parser, Bracket::CLOSE_ROUND)) {
        if(check_token(token_parser, Punctuation::COMMA)) {
          token_parser.pop();
          continue;
        }
        auto next_token = token_parser.peek();
        auto& next_value = next_token.get_value();
        token_parser.pop();
        if(auto property = parse_function(token_parser, next_token)) {
          values.push_back(*property);
        } else if(check_length_property(token_parser, next_token) ||
            check_time_property(token_parser, next_token)) {
          values.push_back(next_token.get_value());
          values.push_back(token_parser.peek().get_value());
          token_parser.pop();
        } else {
          values.push_back(next_value);
        }
      }
      if(!check_token(token_parser, Bracket::CLOSE_ROUND)) {
        throw std::runtime_error("Expected ')' at the end of the function");
      }
      token_parser.pop();
      if(auto converter =
          PropertyRegistry::get_instance()->get_function_converter(name)) {
        return converter(values);
      }
    }
    return none;
  }
}

void Spire::register_property_converter(const std::string& name,
    PropertyConverter converter) {
  PropertyRegistry::get_instance()->register_converter(name,
    std::move(converter));
}

void Spire::register_function_converter(const std::string& name,
    FunctionConverter converter) {
  PropertyRegistry::get_instance()->register_function_converter(name,
    std::move(converter));
}

void Spire::parse_block(TokenParser& token_parser, Rule& rule) {
  if(token_parser.get_size() == 0) {
    throw std::runtime_error("Unexpected end of tokens.");
  }
  if(!check_token(token_parser, Bracket::OPEN_CURLY)) {
    throw std::runtime_error("Expected '{' at the beginning of the block.");
  }
  token_parser.pop();
  while(!check_token(token_parser, Bracket::CLOSE_CURLY)) {
    auto token = token_parser.peek();
    auto& value = token.get_value();
    token_parser.pop();
    auto property_name = Identifier();
    auto values = std::vector<PropertyValue>();
    if(value.type() == typeid(Identifier) &&
        check_adjacent_token(token_parser, token, Punctuation::COLON)) {
      token_parser.pop();
      property_name = boost::get<Identifier>(value);
      while(!check_token(token_parser, Punctuation::SEMI_COLON) &&
          !check_token(token_parser, Bracket::CLOSE_CURLY)) {
        auto next_token = token_parser.peek();
        token_parser.pop();
        if(auto property = parse_function(token_parser, next_token)) {
          values.push_back(*property);
        } else if(check_length_property(token_parser, next_token) ||
            check_time_property(token_parser, next_token)) {
          values.push_back(next_token.get_value());
          values.push_back(token_parser.peek().get_value());
          token_parser.pop();
        } else {
          values.push_back(next_token.get_value());
        }
      }
      if(check_token(token_parser, Punctuation::SEMI_COLON) ||
          check_token(token_parser, Bracket::CLOSE_CURLY)) {
        if(check_token(token_parser, Punctuation::SEMI_COLON)) {
          token_parser.pop();
        }
        if(auto converter =
            PropertyRegistry::get_instance()->get_converter(property_name)) {
          insert(rule, converter(values));
        }
      }
    } else {
      throw std::runtime_error("Expected property name followed by ':'.");
    }
  }
  if(!check_token(token_parser, Bracket::CLOSE_CURLY)) {
    throw std::runtime_error("Expected '}' at the end of the block.");
  }
  token_parser.pop();
}
