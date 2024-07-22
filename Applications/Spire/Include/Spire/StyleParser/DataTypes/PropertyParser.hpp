#ifndef SPIRE_PROPERTY_PARSER_HPP
#define SPIRE_PROPERTY_PARSER_HPP
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include "Spire/Styles/Rule.hpp"

namespace Spire {

  /** Represents the type of property value for parsing. */
  using PropertyValue = boost::variant<Token::Type, Styles::Property>;

  /**
   * The type of function used to convert a list of values to a list of
   * properties.
   * @param values A list of values converted to properties.
   */
  using PropertyConverter =
    std::function<std::vector<Styles::Property> (
      const std::vector<PropertyValue>& values)>;

  /**
   * The type of function used to convert a list of function parameters to
   * a property.
   * @param parameters A list of parameters converted to a property.
   */
  using FunctionConverter =
    std::function<Styles::Property (
      const std::vector<PropertyValue>& parameters)>;

  /** Registers the property converter. */
  void register_property_converter(const std::string& name,
    PropertyConverter converter);

  /** Registers the function converter. */
  void register_function_converter(const std::string& name,
    FunctionConverter converter);
  
  /**
   * Parses a block of properties.
   * @param parser The token parser.
   * @param rule The styling rule consisting of a selector and a block of properties.
   * @throws <code>std::runtime_error</code> iff the property can't be parsed.
   */
  void parse_block(TokenParser& token_parser, Styles::Rule& rule);
}

#endif
