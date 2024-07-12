#include "Spire/StyleParserTests/StyleParserTester.hpp"
#include "Spire/StyleParser/DataTypes/FloatType.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/PropertyParser.hpp"
#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include "Spire/StyleParser/DataTypes/StringType.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  optional<int> parse_number(const Token::Type& value) {
    auto& literal = boost::get<Literal>(value);
    if(*literal.get_type() == IntegerType()) {
      return std::stoi(literal.get_value());
    }
    return none;
  }

  optional<int> convert_length(const Token::Type& value,
      const Token::Type& unit) {
    if(value.type() == typeid(Literal) && unit.type() == typeid(Keyword) &&
        boost::get<Keyword>(unit) == Keyword::PX) {
      return parse_number(value);
    }
    return none;
  }

  template<typename T>
  auto convert_length_property(const std::vector<PropertyValue>& values) {
    auto properties = std::vector<Property>();
    if(values.size() == 2 && values[0].type() == typeid(Token::Type) &&
        values[1].type() == typeid(Token::Type)) {
      if(auto length = convert_length(boost::get<Token::Type>(values[0]),
          boost::get<Token::Type>(values[1]))) {
        properties.push_back(T(*length));
      }
    }
    return properties;
  }

  template<auto T>
  auto convert_length_property(const std::vector<PropertyValue>& values) {
    auto properties = std::vector<Property>();
    if(values.size() == 2 && values[0].type() == typeid(Token::Type) &&
        values[1].type() == typeid(Token::Type)) {
      if(auto length = convert_length(boost::get<Token::Type>(values[0]),
          boost::get<Token::Type>(values[1]))) {
          for_each(T(*length), [&] (auto& property) {
            properties.push_back(property);
          });
      }
    }
    return properties;
  }

  optional<QColor> convert_hexadecimal_color(const Token::Type& value) {
    if(value.type() == typeid(Literal)) {
      auto& literal = boost::get<Literal>(value);
      if(*literal.get_type() == IntegerType() &&
        literal.get_value().substr(0, 2) == "0x") {
        return QColor(std::stoi(literal.get_value().substr(2), 0, 16));
      }
    }
    return none;
  }

  optional<QFont::Weight> convert_font_weight(const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "normal") {
        return QFont::Normal;
      } else if(identifier == "medium") {
        return QFont::Medium;
      } else if(identifier == "bold") {
        return QFont::Bold;
      }
      return QFont::Normal;
    }
    return none;
  }

  optional<std::string> convert_string(const Token::Type& value) {
    if(value.type() == typeid(Literal)) {
      auto& literal = boost::get<Literal>(value);
      if(*literal.get_type() == StringType()) {
        auto text = literal.get_value();
        text.erase(std::remove(text.begin(), text.end(), '"'), text.end());
        return text;
      }
    }
    return none;
  }

  optional<QColor> convert_color(const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "white") {
        return QColor(Qt::white);
      } else if(identifier == "black") {
        return QColor(Qt::black);
      } else if(identifier == "transparent") {
        return QColor(Qt::transparent);
      }
      return QColor();
    }
    return none;
  }

  optional<time_duration> convert_time_duration(const Token::Type& value,
      const Token::Type& unit) {
    if(value.type() != typeid(Literal) || unit.type() != typeid(Keyword)) {
      return none;
    }
    if(boost::get<Keyword>(unit) == Keyword::MS) {
      if(auto number = parse_number(value)) {
        return milliseconds(*number);
      }
    } else if(boost::get<Keyword>(unit) == Keyword::S) {
      if(auto number = parse_number(value)) {
        return seconds(*number);
      }
    } else if(boost::get<Keyword>(unit) == Keyword::MIN) {
      if(auto number = parse_number(value)) {
        return minutes(*number);
      }
    }
    return none;
  }
}

void Spire::StyleParser::Tests::register_selection_types() {
  register_state_selector("disabled", Disabled());
  register_state_selector("hover", Hover());
  register_state_selector("focus", Focus());
  register_state_selector("focus_in", FocusIn());
  register_state_selector("checked", Checked());
  register_state_selector("press", Press());
  register_state_selector("pop_up", PopUp());
  register_state_selector("body", Body());
  register_state_selector("row", Row());
  register_type_selector("Button", is_a<Button>());
  register_type_selector("Icon", is_a<Icon>());
  register_type_selector("TextBox", is_a<TextBox>());
  register_pseudo_selector("placeholder", Placeholder());
}

void Spire::StyleParser::Tests::register_property_converters() {
  register_property_converter("background_color",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      if(values.size() == 1) {
        if(values[0].type() == typeid(Token::Type)) {
          if(auto color = convert_hexadecimal_color(
              boost::get<Literal>(boost::get<Token::Type>(values[0])))) {
            properties.push_back(BackgroundColor(*color));
          }
        } else if(values[0].type() == typeid(Property)) {
          properties.push_back(BackgroundColor(
            boost::get<Property>(values[0]).expression_as<QColor>()));
        }
      }
      return properties;
    });

  register_property_converter("border_top_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderTopSize>(values);
    });

  register_property_converter("border_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<border_size>(values);
    });

  register_property_converter("padding_left",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingLeft>(values);
    });

  register_property_converter("padding_right",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingRight>(values);
    });

  register_property_converter("text_style",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      auto font = QFont();
      if(values.size() == 5) {
        for(auto& value : values) {
          if(value.type() != typeid(Token::Type)) {
            return properties;
          }
        }
        if(auto weight = convert_font_weight(boost::get<Identifier>(
            boost::get<Token::Type>(values[0])))) {
          font.setWeight(*weight);
        }
        if(auto length = convert_length(boost::get<Token::Type>(values[1]),
            boost::get<Token::Type>(values[2]))) {
          font.setPixelSize(*length);
        }
        if(auto family = convert_string(boost::get<Token::Type>(values[3]))) {
          font.setFamily(QString::fromStdString(*family));
        }
        auto color = convert_color(boost::get<Identifier>(
          boost::get<Token::Type>(values[4])));
        if(!color) {
          return properties;
        }
        auto composite_property = TextStyle(font, *color);
        for_each(composite_property, [&] (auto& property) {
          properties.push_back(property);
        });
      }
      return properties;
    });

  register_function_converter("timeout",
    [] (const std::vector<PropertyValue>& values) {
      if(values.size() == 3 && values[0].type() == typeid(Token::Type) &&
          values[1].type() == typeid(Token::Type) &&
          values[2].type() == typeid(Token::Type)) {
        auto color = convert_hexadecimal_color(
          boost::get<Token::Type>(values[0]));
        auto duration = convert_time_duration(
          boost::get<Token::Type>(values[1]),
          boost::get<Token::Type>(values[2]));
        if(color && duration) {
          return BasicProperty<QColor, void>(Expression<QColor>(
            TimeoutExpression(*color, *duration)));
        }
      }
      throw std::runtime_error("Cannot parse the timeout function.");
    });

  register_function_converter("chain",
    [] (const std::vector<PropertyValue>& values) {
      if(values.size() == 2) {
        if(values[0].type() == typeid(Property) &&
            values[1].type() == typeid(Token::Type)) {
          auto& property = boost::get<Property>(values[0]);
          auto& token = boost::get<Token::Type>(values[1]);
          if(property.get_type() == typeid(BasicProperty<QColor, void>)) {
            auto expression =
              property.as<BasicProperty<QColor, void>>().get_expression();
            if(token.type() == typeid(Identifier)) {
              auto& identifier = boost::get<Identifier>(token);
              if(identifier == "revert") {
                return BasicProperty<QColor, void>(Expression<QColor>(
                  ChainExpression(expression, revert)));
              }
            }
          }
        }
      }
      throw std::runtime_error("Cannot parse the chain function.");
    });
}
