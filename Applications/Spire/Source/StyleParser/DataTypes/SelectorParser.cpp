#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include <boost/algorithm/string.hpp>
#include "Spire/StyleParser/DataTypes/Token.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost;
using namespace boost::algorithm;
using namespace Spire;
using namespace Styles;

namespace {
  template<typename T>
  optional<T> check_next_token(const TokenParser& token_parser) {
    if(token_parser.get_size() > 0) {
      auto& value = token_parser.peek().get_value();
      if(value.type() == typeid(T)) {
        return boost::get<T>(value);
      }
    }
    return none;
  }

  template<typename T>
  auto check_next_token(const TokenParser& token_parser, T token) {
    if(auto next = check_next_token<T>(token_parser)) {
      return *next == token;
    }
    return false;
  }

  template<typename T>
  optional<T> check_next_consecutive_token(
      const TokenParser& token_parser, const Token& previous_token) {
    if(token_parser.get_size() > 0) {
      auto& token = token_parser.peek();
      auto& value = token.get_value();
      if(token.get_column_number() == previous_token.get_column_number() + 1 &&
          token.get_line_number() == previous_token.get_line_number() &&
          value.type() == typeid(T)) {
        return boost::get<T>(value);
      }
    }
    return none;
  }

  template<typename T>
  auto check_next_consecutive_token(const TokenParser& token_parser,
      const Token& previous_token, T token) {
    if(auto next = check_next_consecutive_token<T>(token_parser,
        previous_token)) {
      return *next == token;
    }
    return false;
  }

  class SelectorTypeRegistry {
    public:

      static std::shared_ptr<SelectorTypeRegistry> get_instance() {
        static auto instance = std::make_shared<SelectorTypeRegistry>();
        return instance;
      }

      void register_type_selector(const std::string& name,
          const Selector& selector) {
        m_type_selectors[to_lower_copy(name)] = selector;
      }

      void register_state_selector(const std::string& name,
          const Selector& selector) {
        m_state_selectors[to_lower_copy(name)] = selector;
      }

      void register_pseudo_selector(const std::string& name,
          const Selector& selector) {
        m_pseudo_selectors[to_lower_copy(name)] = selector;
      }

      optional<Selector> get_type_selector(const std::string& name) const {
        if(auto i = m_type_selectors.find(name); i != m_type_selectors.end()) {
          return i->second;
        }
        return none;
      }

      optional<Selector> get_state_selector(const std::string& name) const {
        if(auto i = m_state_selectors.find(name);
            i != m_state_selectors.end()) {
          return i->second;
        }
        return none;
      }

      optional<Selector> get_pseudo_selector(const std::string& name) const {
        if(auto i = m_pseudo_selectors.find(name);
            i != m_pseudo_selectors.end()) {
          return i->second;
        }
        return none;
      }

    private:
      std::unordered_map<std::string, optional<Selector>> m_type_selectors;
      std::unordered_map<std::string, optional<Selector>> m_state_selectors;
      std::unordered_map<std::string, optional<Selector>> m_pseudo_selectors;
  };
}

Selector AnySelectorParser::parse() {
  return Any();
}

TypeSelectorParser::TypeSelectorParser(Identifier identifier)
  : m_identifier(to_lower_copy(std::move(identifier))) {}

Selector TypeSelectorParser::parse() {
  if(auto selector = SelectorTypeRegistry::get_instance()->get_type_selector(
      m_identifier)) {
    return *selector;
  }
  throw std::runtime_error("Invalid type identifier.");
}

StateSelectorParser::StateSelectorParser(Identifier identifier)
  : m_identifier(to_lower_copy(std::move(identifier))) {}

Selector StateSelectorParser::parse() {
  if(auto selector = SelectorTypeRegistry::get_instance()->get_state_selector(
      m_identifier)) {
    return *selector;
  }
  throw std::runtime_error("Invalid state identifier.");
}

PseudoElementParser::PseudoElementParser(Identifier identifier)
  : m_identifier(to_lower_copy(std::move(identifier))) {}

Selector PseudoElementParser::parse() {
  if(auto selector = SelectorTypeRegistry::get_instance()->get_pseudo_selector(
      m_identifier)) {
    return *selector;
  }
  throw std::runtime_error("Invalid pseudo element identifier.");
}

PropertyMatchSelectorParser::PropertyMatchSelectorParser(Keyword property,
  Keyword value)
  : m_property(std::move(property)),
    m_value(std::move(value)) {}

Selector PropertyMatchSelectorParser::parse() {
  if(m_property == Keyword::VISIBILITY) {
    if(m_value == Keyword::NONE) {
      return PropertyMatchSelector(Visibility::NONE);
    } else if(m_value == Keyword::VISIBLE) {
      return PropertyMatchSelector(Visibility::VISIBLE);
    } else if(m_value == Keyword::INVISIBLE) {
      return PropertyMatchSelector(Visibility::INVISIBLE);
    }
  }
  throw std::runtime_error("Invalid property.");
}

ChildSelectorParser::ChildSelectorParser(
  std::shared_ptr<SelectorParser> base, std::shared_ptr<SelectorParser> child)
  : m_base(std::move(base)),
    m_child(std::move(child)) {}

Selector ChildSelectorParser::parse() {
  return ChildSelector(m_base->parse(), m_child->parse());
}

DescendantSelectorParser::DescendantSelectorParser(
  std::shared_ptr<SelectorParser> base,
  std::shared_ptr<SelectorParser> descendant)
  : m_base(std::move(base)),
    m_descendant(std::move(descendant)) {}

Selector DescendantSelectorParser::parse() {
  return DescendantSelector(m_base->parse(), m_descendant->parse());
}

ParentSelectorParser::ParentSelectorParser(
  std::shared_ptr<SelectorParser> base, std::shared_ptr<SelectorParser> parent)
  : m_base(std::move(base)),
    m_parent(std::move(parent)) {}

Selector ParentSelectorParser::parse() {
  return ParentSelector(m_base->parse(), m_parent->parse());
}

AncestorSelectorParser::AncestorSelectorParser(
  std::shared_ptr<SelectorParser> base, std::shared_ptr<SelectorParser> ancestor)
  : m_base(std::move(base)),
    m_ancestor(std::move(ancestor)) {}

Selector AncestorSelectorParser::parse() {
  return AncestorSelector(m_base->parse(), m_ancestor->parse());
}

SiblingSelectorParser::SiblingSelectorParser(
  std::shared_ptr<SelectorParser> base, std::shared_ptr<SelectorParser> sibling)
  : m_base(std::move(base)),
    m_sibling(std::move(sibling)) {}

Selector SiblingSelectorParser::parse() {
  return SiblingSelector(m_base->parse(), m_sibling->parse());
}

AndSelectorParser::AndSelectorParser(
  std::shared_ptr<SelectorParser> left, std::shared_ptr<SelectorParser> right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

Selector AndSelectorParser::parse() {
  return AndSelector(m_left->parse(), m_right->parse());
}

OrSelectorParser::OrSelectorParser(
  std::shared_ptr<SelectorParser> left, std::shared_ptr<SelectorParser> right)
  : m_left(std::move(left)),
    m_right(std::move(right)) {}

Selector OrSelectorParser::parse() {
  return OrSelector(m_left->parse(), m_right->parse());
}

NotSelectorParser::NotSelectorParser(std::shared_ptr<SelectorParser> operand)
  : m_operand(std::move(operand)) {}

Selector NotSelectorParser::parse() {
  return NotSelector(m_operand->parse());
}

FlipSelectorParser::FlipSelectorParser(std::shared_ptr<SelectorParser> operand)
  : m_operand(std::move(operand)) {}

Selector FlipSelectorParser::parse() {
  return FlipSelector(m_operand->parse());
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_expression(
    TokenParser& token_parser) {
  return parse_or(token_parser);
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_primary(
    TokenParser& token_parser) {
  if(token_parser.get_size() == 0) {
    throw std::runtime_error("Unexpected end of tokens.");
  }
  auto token = token_parser.peek();
  auto& value = token.get_value();
  token_parser.pop();
  if(value.type() == typeid(Bracket)) {
    auto& bracket = boost::get<Bracket>(value);
    if(bracket == Bracket::OPEN_ROUND) {
      auto expression = parse_expression(token_parser);
      if(!check_next_token(token_parser, Bracket::CLOSE_ROUND)) {
        throw std::runtime_error("Expected ')'.");
      }
      token_parser.pop();
      return expression;
    } else if(bracket == Bracket::OPEN_SQUARE) {
      return parse_property(token_parser);
    }
  } else if(value.type() == typeid(Punctuation)) {
    auto& punctuation = boost::get<Punctuation>(value);
    if(punctuation == Punctuation::COLON) {
      return parse_colon_sign(token_parser, token);
    }
  } else if(value.type() == typeid(Keyword)) {
    auto& keyword = boost::get<Keyword>(value);
    if(keyword == Keyword::ANY) {
      return std::make_shared<AnySelectorParser>();
    } else if(keyword == Keyword::EXCLAMATION || keyword == Keyword::TILDE) {
      auto& next_token = token_parser.peek();
      if(next_token.get_line_number() == token.get_line_number() &&
          next_token.get_column_number() == token.get_column_number() + 1) {
        auto parser = parse_primary(token_parser);
        if(keyword == Keyword::EXCLAMATION) {
          return std::make_shared<NotSelectorParser>(parser);
        }
        return std::make_shared<FlipSelectorParser>(parser);
      }
      throw std::runtime_error("Invalid the selector.");
    }
  } else if(value.type() == typeid(Identifier)) {
    auto& identifier = boost::get<Identifier>(value);
    if(check_next_token(token_parser, Bracket::OPEN_SQUARE)) {
      token_parser.pop();
      return std::make_shared<AndSelectorParser>(
        std::make_shared<TypeSelectorParser>(identifier),
        parse_property(token_parser));
    }
    return std::make_shared<TypeSelectorParser>(identifier);
  }
  throw std::runtime_error("Unknown token.");
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_or(
    TokenParser& token_parser) {
  auto left = parse_and(token_parser);
  while(check_next_token(token_parser, Keyword::OR)) {
    token_parser.pop();
    left = std::make_shared<OrSelectorParser>(left, parse_and(token_parser));
  }
  return left;
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_and(
    TokenParser& token_parser) {
  auto left = parse_hierarchy(token_parser);
  while(check_next_token(token_parser, Keyword::AND)) {
    token_parser.pop();
    left = std::make_shared<AndSelectorParser>(left,
      parse_hierarchy(token_parser));
  }
  return left;
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_hierarchy(
    TokenParser& token_parser) {
  auto left = parse_primary(token_parser);
  while(token_parser.get_size() > 0) {
    if(check_next_token(token_parser, Keyword::LESS_THAN)) {
      auto token = token_parser.peek();
      token_parser.pop();
      if(check_next_consecutive_token(token_parser, token,
          Keyword::LESS_THAN)) {
        token_parser.pop();
        left = std::make_shared<AncestorSelectorParser>(left,
          parse_primary(token_parser));
      } else {
        left = std::make_shared<ParentSelectorParser>(left,
          parse_primary(token_parser));
      }
    } else if(check_next_token(token_parser, Keyword::GREATER_THAN)) {
      auto token = token_parser.peek();
      token_parser.pop();
      if(check_next_consecutive_token(token_parser, token,
          Keyword::GREATER_THAN)) {
        token_parser.pop();
        left = std::make_shared<DescendantSelectorParser>(left,
          parse_primary(token_parser));
      } else {
        left = std::make_shared<ChildSelectorParser>(left,
          parse_primary(token_parser));
      }
    } else if(check_next_token(token_parser, Keyword::PERCENTAGE)) {
      token_parser.pop();
      left = std::make_shared<SiblingSelectorParser>(left,
        parse_primary(token_parser));
    } else {
      break;
    }
  }
  return left;
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_property(
    TokenParser& token_parser) {
  auto property_name = [&] {
    if(auto next = check_next_token<Keyword>(token_parser)) {
      return *next;
    }
    throw std::runtime_error("Expected property name.");
  }();
  token_parser.pop();
  auto is_not = false;
  auto token = token_parser.peek();
  if(check_next_token<Keyword>(token_parser, Keyword::EXCLAMATION)) {
    is_not = true;
    token_parser.pop();
  }
  if(is_not) {
    if(!check_next_consecutive_token(token_parser, token, Keyword::EQUAL)) {
      throw std::runtime_error("Expected '='.");
    }
  } else {
    if(!check_next_token(token_parser, Keyword::EQUAL)) {
      throw std::runtime_error("Expected '='.");
    }
  }
  token_parser.pop();
  auto property_value = [&] {
    if(auto next = check_next_token<Keyword>(token_parser)) {
      return *next;
    }
    throw std::runtime_error("Expected property value.");
  }();
  token_parser.pop();
  if(!check_next_token(token_parser, Bracket::CLOSE_SQUARE)) {
    throw std::runtime_error("Expected ']'.");
  }
  token_parser.pop();
  if(is_not) {
    return std::make_shared<NotSelectorParser>(
      std::make_shared<PropertyMatchSelectorParser>(property_name,
        property_value));
  }
  return std::make_shared<PropertyMatchSelectorParser>(property_name,
    property_value);
}

std::shared_ptr<SelectorParser> DefaultSelectorParseStrategy::parse_colon_sign(
    TokenParser& token_parser, const Token& token) {
  if(token_parser.get_size() == 0) {
    throw std::runtime_error("Unexpected end of tokens.");
  }
  if(auto next = check_next_consecutive_token<Identifier>(token_parser,
      token)) {
    token_parser.pop();
    return std::make_shared<StateSelectorParser>(*next);
  } else if(check_next_consecutive_token(token_parser, token,
      Punctuation::COLON)) {
    auto colon_token = token_parser.peek();
    token_parser.pop();
    if(auto next = check_next_consecutive_token<Identifier>(token_parser,
        colon_token)) {
      return std::make_shared<PseudoElementParser>(*next);
    }
  }
  throw std::runtime_error("Invalid colon sign.");
}

void Spire::register_type_selector(const std::string& name,
    const Selector& selector) {
  SelectorTypeRegistry::get_instance()->register_type_selector(name, selector);
}

void Spire::register_state_selector(const std::string& name,
    const Selector& selector) {
  SelectorTypeRegistry::get_instance()->register_state_selector(name, selector);
}

void Spire::register_pseudo_selector(const std::string& name,
    const Selector& selector) {
  SelectorTypeRegistry::get_instance()->register_pseudo_selector(name,
    selector);
}

Selector Spire::parse_selector(TokenParser& token_parser,
    std::shared_ptr<SelectorParseStrategy> strategy) {
  return strategy->parse_expression(token_parser)->parse();
}
