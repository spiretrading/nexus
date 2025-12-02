#include "Spire/UiViewer/StyleParserEnvironment.hpp"
#include <span>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/StyleParser/DataTypes/FloatType.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/PropertyParser.hpp"
#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include "Spire/StyleParser/DataTypes/StringType.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/HighlightPicker.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/Slider.hpp"
#include "Spire/Ui/Slider2D.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableHeader.hpp"
#include "Spire/Ui/TableHeaderItem.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  optional<bool> convert_boolean(const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "true") {
        return true;
      } else if(identifier == "false") {
        return false;
      }
    }
    return none;
  }

  optional<int> convert_number(const Token::Type& value) {
    if(value.type() == typeid(Literal)) {
      auto& literal = boost::get<Literal>(value);
      if(*literal.get_type() == IntegerType()) {
        return std::stoi(literal.get_value());
      }
    }
    return none;
  }

  template<typename T>
  auto conver_number_property(std::span<const PropertyValue> values) {
    auto properties = std::vector<Property>();
    if(values.size() == 1 && values[0].type() == typeid(Token::Type)) {
      if(auto number = convert_number(boost::get<Token::Type>(values[0]))) {
        properties.push_back(T(*number));
      }
    }
    return properties;
  }

  optional<int> convert_length(const Token::Type& value,
      const Token::Type& unit) {
    if(value.type() == typeid(Literal) && unit.type() == typeid(Keyword) &&
      boost::get<Keyword>(unit) == Keyword::PX) {
      return convert_number(value);
    }
    return none;
  }

  optional<int> convert_length(const Token::Type& value) {
    return convert_number(value);
  }

  optional<int> convert_length(std::span<const PropertyValue> values) {
    if(values.size() == 2 && values[0].type() == typeid(Token::Type) &&
        values[1].type() == typeid(Token::Type)) {
      return convert_length(boost::get<Token::Type>(values[0]),
        boost::get<Token::Type>(values[1]));
    } else if(values.size() == 1 && values[0].type() == typeid(Token::Type)) {
      if(auto length = convert_length(boost::get<Token::Type>(values[0]));
          length && *length == 0) {
        return length;
      }
    }
    return none;
  }

  template<typename T>
  auto convert_length_property(std::span<const PropertyValue> values,
      Qt::Orientation orientation) {
    auto properties = std::vector<Property>();
    if(auto length = convert_length(values)) {
      if(orientation == Qt::Horizontal) {
        properties.push_back(T(scale_width(*length)));
      } else {
        properties.push_back(T(scale_height(*length)));
      }
    }
    return properties;
  }

  template<typename F>
  auto convert_composite_length_property(
      std::span<const PropertyValue> values, F&& f) {
    auto properties = std::vector<Property>();
    if(auto length = convert_length(values)) {
      for_each(f(*length), [&] (auto& property) {
        properties.push_back(property);
      });
    }
    return properties;
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

  optional<QColor> convert_predefined_color(const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "white") {
        return QColor(Qt::white);
      } else if(identifier == "black") {
        return QColor(Qt::black);
      } else if(identifier == "red") {
        return QColor(Qt::red);
      } else if(identifier == "green") {
        return QColor(Qt::green);
      } else if(identifier == "blue") {
        return QColor(Qt::blue);
      } else if(identifier == "gray") {
        return QColor(Qt::gray);
      } else if(identifier == "cyan") {
        return QColor(Qt::cyan);
      } else if(identifier == "magenta") {
        return QColor(Qt::magenta);
      } else if(identifier == "yellow") {
        return QColor(Qt::yellow);
      } else if(identifier == "transparent") {
        return QColor(Qt::transparent);
      }
      return QColor();
    }
    return none;
  }

  optional<QColor> convert_color(const Token::Type& value) {
    if(auto color = convert_hexadecimal_color(value)) {
      return color;
    } else if(auto color = convert_predefined_color(value)) {
      return color;
    }
    return none;
  }

  template<typename T>
  auto convert_color_property(std::span<const PropertyValue> values) {
    auto properties = std::vector<Property>();
    if(values.size() == 1) {
      if(values[0].type() == typeid(Token::Type)) {
        if(auto color = convert_color(boost::get<Token::Type>(values[0]))) {
          properties.push_back(T(*color));
        }
      } else if(values[0].type() == typeid(Property)) {
        properties.push_back(
          T(boost::get<Property>(values[0]).expression_as<QColor>()));
      }
    }
    return properties;
  }

  template<typename F>
  auto convert_composite_color_property_only_color(
      std::span<const PropertyValue> values, F&& f) {
    auto properties = std::vector<Property>();
    if(values.size() == 1 && values[0].type() == typeid(Token::Type)) {
      if(auto color = convert_color(boost::get<Token::Type>(values[0]))) {
        for_each(f(*color), [&] (auto& property) {
          properties.push_back(property);
        });
      }
    }
    return properties;
  }

  template<typename F>
  auto convert_composite_color_property(
      std::span<const PropertyValue> values, F&& f) {
    auto properties = std::vector<Property>();
    auto color_properties =
      convert_composite_color_property_only_color(values, f);
    if(!color_properties.empty()) {
      properties = std::move(color_properties);
    } else if(values.size() == 1 && values[0].type() == typeid(Property)) {
      for_each(f(boost::get<Property>(values[0]).expression_as<QColor>()),
        [&] (auto& property) {
          properties.push_back(property);
        });
    }
    return properties;
  }

  optional<QFont::Weight> convert_predefined_font_weight(
      const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "normal") {
        return QFont::Normal;
      } else if(identifier == "medium") {
        return QFont::Medium;
      } else if(identifier == "bold") {
        return QFont::Bold;
      } else if(identifier == "thin") {
        return QFont::Thin;
      } else if(identifier == "extra_light") {
        return QFont::ExtraLight;
      } else if(identifier == "light") {
        return QFont::Light;
      } else if(identifier == "demi_bold") {
        return QFont::DemiBold;
      } else if(identifier == "extra_bold") {
        return QFont::ExtraBold;
      } else if(identifier == "black") {
        return QFont::Black;
      }
    }
    return none;
  }

  optional<int> convert_numerical_font_weight(const Token::Type& value) {
    if(auto number = convert_number(value)) {
      if(*number > 0 && *number < 1000) {
        return *number / 10;
      }
    }
    return none;
  }

  optional<QFont> convert_font(std::span<const PropertyValue> values) {
    for(auto& value : values) {
      if(value.type() != typeid(Token::Type)) {
        return none;
      }
    }
    auto font = QFont();
    auto convert_weight = [&] (const Token::Type& weight) {
      if(auto font_weight = convert_predefined_font_weight(weight)) {
        font.setWeight(*font_weight);
      } else if(auto font_weight = convert_numerical_font_weight(weight)) {
        font.setWeight(*font_weight);
      }
    };
    auto convert_family = [&] (const Token::Type& family) {
      if(auto font_family = convert_string(family)) {
        font.setFamily(QString::fromStdString(*font_family));
      }
    };
    if(values.size() == 4) {
      convert_weight(boost::get<Token::Type>(values[0]));
      if(auto length =
         convert_length({values.begin() + 1, values.begin() + 3})) {
        font.setPixelSize(scale_width(*length));
      }
      convert_family(boost::get<Token::Type>(values[3]));
      return font;
    } else if(values.size() == 3) {
      convert_weight(boost::get<Token::Type>(values[0]));
      if(auto length =
          convert_length({values.begin() + 1, values.begin() + 2})) {
        font.setPixelSize(scale_width(*length));
      }
      convert_family(boost::get<Token::Type>(values[2]));
      return font;
    }
    return none;
  }

  optional<Qt::Alignment> convert_alignment(const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "left") {
        return Qt::Alignment(Qt::AlignLeft);
      } else if(identifier == "right") {
        return Qt::Alignment(Qt::AlignRight);
      } else if(identifier == "top") {
        return Qt::Alignment(Qt::AlignTop);
      } else if(identifier == "bottom") {
        return Qt::Alignment(Qt::AlignBottom);
      } else if(identifier == "center") {
        return Qt::Alignment(Qt::AlignCenter);
      }
    }
    return none;
  }

  optional<time_duration> convert_time_duration(const Token::Type& value,
      const Token::Type& unit) {
    if(value.type() != typeid(Literal) || unit.type() != typeid(Keyword)) {
      return none;
    }
    if(boost::get<Keyword>(unit) == Keyword::MS) {
      if(auto number = convert_number(value)) {
        return milliseconds(*number);
      }
    } else if(boost::get<Keyword>(unit) == Keyword::S) {
      if(auto number = convert_number(value)) {
        return seconds(*number);
      }
    } else if(boost::get<Keyword>(unit) == Keyword::MIN) {
      if(auto number = convert_number(value)) {
        return minutes(*number);
      }
    }
    return none;
  }

  optional<RevertPolymorph> convert_revert(const Token::Type& value) {
    if(value.type() == typeid(Identifier)) {
      auto& identifier = boost::get<Identifier>(value);
      if(identifier == "revert") {
        return revert;
      }
    }
    return none;
  }

  optional<BasicProperty<QColor, void>::Expression> convert_color_expression(
      const Property& property) {
    if(property.get_type() == typeid(BasicProperty<QColor, void>)) {
      return property.as<BasicProperty<QColor, void>>().get_expression();
    }
    return none;
  };
}

void Spire::register_selectors() {
  register_state_selector("active", Active());
  register_state_selector("disabled", Disabled());
  register_state_selector("hover", Hover());
  register_state_selector("focus", Focus());
  register_state_selector("focus_in", FocusIn());
  register_state_selector("focus_visible", FocusVisible());
  register_state_selector("drag", Drag());
  register_state_selector("checked", Checked());
  register_state_selector("press", Press());
  register_state_selector("rejected", Rejected());
  register_state_selector("pop_up", PopUp());
  register_state_selector("body", Body());
  register_state_selector("row", Row());
  register_state_selector("current_row", CurrentRow());
  register_state_selector("column", Column());
  register_state_selector("current_column", CurrentColumn());
  register_state_selector("hover_item", HoverItem());
  register_state_selector("read_only", ReadOnly());
  register_state_selector("is_positive", IsPositive());
  register_state_selector("is_negative", IsNegative());
  register_state_selector("uptick", Uptick());
  register_state_selector("downtick", Downtick());
  register_state_selector("label", TableHeaderItem::Label());
  register_state_selector("sortable", TableHeaderItem::Sortable());
  register_state_selector("filtered", TableHeaderItem::Filtered());
  register_state_selector("filter_button", TableHeaderItem::FilterButton());
  register_state_selector("hover_element", TableHeaderItem::HoverElement());
  register_state_selector("prompt", Prompt());
  register_state_selector("colon", Colon());
  register_state_selector("track", Track());
  register_state_selector("thumb", Thumb());
  register_state_selector("track_rail", TrackRail());
  register_state_selector("track_fill", TrackFill());
  register_state_selector("track_pad", TrackPad());
  register_type_selector("Box", is_a<Box>());
  register_type_selector("Button", is_a<Button>());
  register_type_selector("Icon", is_a<Icon>());
  register_type_selector("TextBox", is_a<TextBox>());
  register_type_selector("TableBody", is_a<TableBody>());
  register_type_selector("TableItem", is_a<TableItem>());
  register_type_selector("TableHeader", is_a<TableHeader>());
  register_type_selector("TableHeaderItem", is_a<TableHeaderItem>());
  register_type_selector("HighlightPicker", is_a<HighlightPicker>());
  register_pseudo_selector("placeholder", Placeholder());
}

void Spire::register_property_converters() {
  register_property_converter("background_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<BackgroundColor>(values);
    });

  register_property_converter("border_top_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderTopSize>(values, Qt::Vertical);
    });

  register_property_converter("border_right_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderRightSize>(values, Qt::Horizontal);
    });

  register_property_converter("border_bottom_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderBottomSize>(values, Qt::Vertical);
    });

  register_property_converter("border_left_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderLeftSize>(values, Qt::Horizontal);
    });

  register_property_converter("border_size",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_length_property(values, [] (int length) {
        return BorderSize(scale_height(length), scale_width(length),
          scale_height(length), scale_width(length));
      });
    });

  register_property_converter("border_top_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<BorderTopColor>(values);
    });

  register_property_converter("border_right_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<BorderRightColor>(values);
    });

  register_property_converter("border_bottom_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<BorderBottomColor>(values);
    });

  register_property_converter("border_left_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<BorderLeftColor>(values);
    });

  register_property_converter("border_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_color_property(values,
        [] (Expression<QColor> color) {
          return border_color(color);
        });
    });

  register_property_converter("border",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      if(values.size() == 3 && values[0].type() == typeid(Token::Type) &&
          values[1].type() == typeid(Token::Type) &&
          values[2].type() == typeid(Token::Type)) {
        auto border_sizes = convert_composite_length_property(
          {values.begin(), values.begin() + 2},
          [] (int length) {
            return BorderSize(scale_height(length), scale_width(length),
              scale_height(length), scale_width(length));
          });
        for(auto& border_size : border_sizes) {
          properties.push_back(border_size);
        }
        if(auto color = convert_color(boost::get<Token::Type>(values[2]))) {
          for_each(border_color(*color), [&] (auto& property) {
            properties.push_back(property);
          });
        }
      }
      return properties;
    });

  register_property_converter("padding_top",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingTop>(values, Qt::Vertical);
    });

  register_property_converter("padding_left",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingLeft>(values, Qt::Horizontal);
    });

  register_property_converter("padding_bottom",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingBottom>(values, Qt::Vertical);
    });

  register_property_converter("padding_right",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingRight>(values, Qt::Horizontal);
    });

  register_property_converter("horizontal_padding",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_length_property(values, [] (int length) {
        return horizontal_padding(scale_width(length));
      });
    });

  register_property_converter("vertical_padding",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_length_property(values, [] (int length) {
        return vertical_padding(scale_height(length));
      });
    });

  register_property_converter("border_top_left_radius",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderTopLeftRadius>(values,
        Qt::Horizontal);
    });

  register_property_converter("border_top_right_radius",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderTopRightRadius>(values,
        Qt::Horizontal);
    });

  register_property_converter("border_bottom_right_radius",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderBottomRightRadius>(values,
        Qt::Horizontal);
    });

  register_property_converter("border_bottom_left_radius",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<BorderBottomLeftRadius>(values,
        Qt::Horizontal);
    });

  register_property_converter("border_radius",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_length_property(values, [] (int length) {
        return border_radius(scale_width(length));
      });
    });

  register_property_converter("padding_right",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<PaddingRight>(values, Qt::Horizontal);
    });

  register_property_converter("horizontal_spacing",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<HorizontalSpacing>(values, Qt::Horizontal);
    });

  register_property_converter("vertical_spacing",
    [] (const std::vector<PropertyValue>& values) {
      return convert_length_property<VerticalSpacing>(values, Qt::Horizontal);
    });

  register_property_converter("spacing",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_length_property(values, [] (int length) {
        return Spacing(scale_width(length), scale_height(length));
      });
    });

  register_property_converter("horizontal_grid_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<HorizontalGridColor>(values);
    });

  register_property_converter("vertical_grid_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<VerticalGridColor>(values);
    });

  register_property_converter("grid_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_composite_color_property_only_color(values,
        [] (const QColor& color) {
          return grid_color(color);
        });
    });

  register_property_converter("leading_zeros",
    [] (const std::vector<PropertyValue>& values) {
      return conver_number_property<LeadingZeros>(values);
    });

  register_property_converter("trailing_zeros",
    [] (const std::vector<PropertyValue>& values) {
      return conver_number_property<TrailingZeros>(values);
    });

  register_property_converter("text_align",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      if(values.size() == 1 && values[0].type() == typeid(Token::Type)) {
        if(auto align = convert_alignment(boost::get<Identifier>(
            boost::get<Token::Type>(values[0])))) {
          properties.push_back(TextAlign(*align));
        }
      }
      return properties;
    });

  register_property_converter("text_color",
    [] (const std::vector<PropertyValue>& values) {
      return convert_color_property<TextColor>(values);
    });

  register_property_converter("font",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      if(auto font = convert_font(values)) {
        properties.push_back(Font(*font));
      }
      return properties;
    });

  register_property_converter("text_style",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      if(values.empty()) {
        return properties;
      }
      auto font = convert_font(std::span(values.begin(), values.end() - 1));
      auto color = convert_color(boost::get<Token::Type>(values[4]));
      if(font && color) {
        auto composite_property = TextStyle(*font, *color);
        for_each(composite_property, [&] (auto& property) {
          properties.push_back(property);
        });
      }
      return properties;
    });

  register_property_converter("year_field",
    [] (const std::vector<PropertyValue>& values) {
      auto properties = std::vector<Property>();
      if(values.size() == 1 && values[0].type() == typeid(Token::Type)) {
        if(auto year_field = convert_boolean(boost::get<Identifier>(
            boost::get<Token::Type>(values[0])))) {
          properties.push_back(YearField(*year_field));
        }
      }
      return properties;
    });

  register_function_converter("timeout",
    [] (const std::vector<PropertyValue>& values) {
      if(values.size() == 3 && values[0].type() == typeid(Token::Type) &&
          values[1].type() == typeid(Token::Type) &&
          values[2].type() == typeid(Token::Type)) {
        auto color = convert_color(boost::get<Token::Type>(values[0]));
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

  register_function_converter("linear",
    [] (const std::vector<PropertyValue>& values) {
      if(values.size() == 4 && values[0].type() == typeid(Token::Type) &&
          values[1].type() == typeid(Token::Type) &&
          values[2].type() == typeid(Token::Type) &&
          values[3].type() == typeid(Token::Type)) {
        auto start_color = convert_color(boost::get<Token::Type>(values[0]));
        auto duration = convert_time_duration(
          boost::get<Token::Type>(values[2]),
          boost::get<Token::Type>(values[3]));
        if(start_color && duration) {
          if(auto revert = convert_revert(boost::get<Token::Type>(values[1]))) {
            return BasicProperty<QColor, void>(Expression<QColor>(
              LinearExpression(*start_color, *revert, *duration)));
          } else if(auto end_color =
              convert_color(boost::get<Token::Type>(values[1]))) {
            return BasicProperty<QColor, void>(Expression<QColor>(
              LinearExpression(*start_color, *end_color, *duration)));
          }
        }
      }
      throw std::runtime_error("Cannot parse the linear function.");
    });

  register_function_converter("chain",
    [] (const std::vector<PropertyValue>& values) {
      if(values.size() == 2) {
        if(values[0].type() == typeid(Property)) {
          if(auto first_expression =
              convert_color_expression(boost::get<Property>(values[0]))) {
            if(values[1].type() == typeid(Token::Type)) {
              if(auto revert = convert_revert(boost::get<Token::Type>(values[1]))) {
                return BasicProperty<QColor, void>(Expression<QColor>(
                  ChainExpression(*first_expression, *revert)));
              }
            } else if(values[1].type() == typeid(Property)) {
              if(auto second_expression =
                  convert_color_expression(boost::get<Property>(values[1]))) {
                return BasicProperty<QColor, void>(Expression<QColor>(
                  ChainExpression(*first_expression, *second_expression)));
              }
            }
          }
        }
      }
      throw std::runtime_error("Cannot parse the chain function.");
    });
}
