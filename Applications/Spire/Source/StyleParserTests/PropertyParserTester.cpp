#include <doctest/doctest.h>
#include "Spire/StyleParser/DataTypes/PropertyParser.hpp"
#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include "Spire/StyleParserTests/StyleParserTester.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::StyleParser::Tests;
using namespace Spire::Styles;

TEST_SUITE("PropertyParser") {
  TEST_CASE("Basic Property") {
    register_property_converters();
    auto styles = R"(
      {
        background_color: 0xFFFFFF;
      }
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto rule = Rule();
    parse_block(parser, rule);
    auto& block = rule.get_block();
    REQUIRE(std::distance(block.begin(), block.end()) == 1);
    REQUIRE(*block.begin() == BackgroundColor(QColor(0xFFFFFF)));
  }

  TEST_CASE("Composite Property") {
    auto styles = R"(
      {
        border_size: 2px;
      }
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto rule = Rule();
    parse_block(parser, rule);
    auto& block = rule.get_block();
    REQUIRE(
      std::distance(block.begin(), block.end()) == 4);
    REQUIRE(*block.begin() == BorderTopSize(2));
    REQUIRE(*(block.begin() + 1) == BorderRightSize(2));
    REQUIRE(*(block.begin() + 2) == BorderBottomSize(2));
    REQUIRE(*(block.begin() + 3) == BorderLeftSize(2));
  }

  TEST_CASE("Composite Property2") {
    auto styles = R"(
      {
        text_style: normal 12px "Roboto" black;
      }
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto rule = Rule();
    auto visits = 0;
    parse_block(parser, rule);
    for(auto& property : rule.get_block()) {
      property.visit([&] (const TextColor& property) {
        auto& expression =
          property.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor(Qt::black));
        ++visits;
      },
      [&] (const Font& property) {
        auto& font = property.get_expression().as<ConstantExpression<QFont>>().
          get_constant();
        REQUIRE(font.family() == "Roboto");
        REQUIRE(font.weight() == QFont::Normal);
        REQUIRE(font.pixelSize() == 12);
        ++visits;
      });
    }
    REQUIRE(visits == 2);
  }

  TEST_CASE("Multiple properties") {
    auto styles = R"(
      {
        background_color: 0xFFFFFF;
        border_top_size: 1px;
        padding_left: 4px;
        padding_right: 8px;
      }
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto rule = Rule();
    auto visits = 0;
    parse_block(parser, rule);
    for(auto& property : rule.get_block()) {
      property.visit([&] (const BackgroundColor& property) {
        auto& expression =
          property.get_expression().as<ConstantExpression<QColor>>();
        REQUIRE(expression.get_constant() == QColor(0xFFFFFF));
        ++visits;
      },
      [&] (const BorderTopSize& property) {
        auto& expression =
          property.get_expression().as<ConstantExpression<int>>();
        REQUIRE(expression.get_constant() == 1);
        ++visits;
      },
      [&] (const PaddingLeft& property) {
        auto& expression =
          property.get_expression().as<ConstantExpression<int>>();
        REQUIRE(expression.get_constant() == 4);
        ++visits;
      },
      [&] (const PaddingRight& property) {
        auto& expression =
          property.get_expression().as<ConstantExpression<int>>();
        REQUIRE(expression.get_constant() == 8);
        ++visits;
      });
    }
    REQUIRE(visits == 4);
  }

  TEST_CASE("Function") {
    auto styles = R"(
      {
        background_color: timeout(0xB71C1C, 500ms);
      }
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto rule = Rule();
    parse_block(parser, rule);
    auto& block = rule.get_block();
    REQUIRE(
      std::distance(block.begin(), block.end()) == 1);
    REQUIRE(*block.begin() == BackgroundColor(Expression<QColor>(
      TimeoutExpression(QColor(0xB71C1C), milliseconds(500)))));
  }

  TEST_CASE("Nested Function") {
    register_property_converters();
    auto styles = R"(
      {
        background_color: chain(timeout(0xB71C1C, 500ms), revert);
      }
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto rule = Rule();
    parse_block(parser, rule);
    auto& block = rule.get_block();
    REQUIRE(std::distance(block.begin(), block.end()) == 1);
    REQUIRE(*block.begin() == BackgroundColor(Expression<QColor>(
      ChainExpression(TimeoutExpression(QColor(0xB71C1C), milliseconds(500)),
        revert))));
  }
}
