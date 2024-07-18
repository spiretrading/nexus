#include <doctest/doctest.h>
#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include "Spire/StyleParserTests/StyleParserTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::StyleParser::Tests;
using namespace Spire::Styles;

TEST_SUITE("SelectorParser") {
  TEST_CASE("Any") {
    register_selectors();
    auto styles = R"(
      any {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == Any());
  }

  TEST_CASE("Nested logical1") {
    auto styles = R"(
      :hover and :focus or :press {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == OrSelector(AndSelector(Hover(), Focus()), Press()));
  }

  TEST_CASE("Nested logical2") {
    auto styles = R"(
      :hover or :focus and :press {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector != AndSelector(OrSelector(Hover(), Focus()), Press()));
    REQUIRE(selector == OrSelector(Hover(), AndSelector(Focus(), Press())));
  }

  TEST_CASE("StateSelector") {
    auto styles = R"(
      :hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == Hover());
  }

  TEST_CASE("PseudoElementSelector") {
    auto styles = R"(
      :disabled > ::Placeholder{}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == ChildSelector(Disabled(), Placeholder()));
  }

  TEST_CASE("NotSelector") {
    auto styles = R"(
      !:hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == NotSelector(Hover()));
  }

  TEST_CASE("FlipSelector") {
    auto styles = R"(
      ~:hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == FlipSelector(Hover()));
  }

  TEST_CASE("ChildSelector") {
    auto styles = R"(
      any > :hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == ChildSelector(Any(), Hover()));
  }

  TEST_CASE("DescendantSelector") {
    auto styles = R"(
      any >> :hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == DescendantSelector(Any(), Hover()));
  }

  TEST_CASE("ParentSelector") {
    auto styles = R"(
      any < :hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == ParentSelector(Any(), Hover()));
  }

  TEST_CASE("AncestorSelector") {
    auto styles = R"(
      any << :hover {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == AncestorSelector(Any(), Hover()));
  }

  TEST_CASE("SiblingSelector") {
    auto styles = R"(
      any > Icon % Button {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      SiblingSelector(ChildSelector(Any(), is_a<Icon>()), is_a<Button>()));
  }

  TEST_CASE("PropertySelector") {
    auto styles = R"(
      [visibility = none] {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == PropertyMatchSelector(Visibility::NONE));
  }

  TEST_CASE("PropertySelector2") {
    auto styles = R"(
      Button[visibility = visible] {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      AndSelector(is_a<Button>(), PropertyMatchSelector(Visibility::VISIBLE)));
  }

  TEST_CASE("Not PropertySelector") {
    auto styles = R"(
      Button[visibility != invisible] {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == AndSelector(is_a<Button>(),
      NotSelector(PropertyMatchSelector(Visibility::INVISIBLE))));
  }

  TEST_CASE("Nested ChildSelector") {
    auto styles = R"(
      any > :hover > :body {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector == ChildSelector(ChildSelector(Any(), Hover()), Body()));
  }

  TEST_CASE("CombinatorSelector1") {
    auto styles = R"(
       (:checked and (:hover or :press)) > Icon {}
     )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      ChildSelector(AndSelector(Checked(), OrSelector(Hover(), Press())),
        is_a<Icon>()));
  }

  TEST_CASE("CombinatorSelector2") {
    auto styles = R"(
      !(~any << :body) {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      NotSelector(AncestorSelector(FlipSelector(Any()), Body())));
  }

  TEST_CASE("CombinatorSelector3") {
    auto styles = R"(
      ~any > Button[visibility != none] {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      ChildSelector(FlipSelector(Any()),
        AndSelector(is_a<Button>(),
          NotSelector(PropertyMatchSelector(Visibility::NONE)))));
  }

  TEST_CASE("CombinatorSelector4") {
    auto styles = R"(
      any > (Icon and !(~any << Icon)) {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      ChildSelector(Any(),
        AndSelector(is_a<Icon>(),
          NotSelector(AncestorSelector(FlipSelector(Any()), is_a<Icon>())))));
  }

  TEST_CASE("CombinatorSelector5") {
    auto styles = R"(
      :pop_up > TextBox or (~any > Button and (:hover or :focus_in)) > TextBox{
    })";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      OrSelector(ChildSelector(PopUp(), is_a<TextBox>()),
        ChildSelector(
          AndSelector(
            ChildSelector(FlipSelector(Any()), is_a<Button>()),
            OrSelector(Hover(), FocusIn())),
          is_a<TextBox>())));
  }

  TEST_CASE("CombinatorSelector6") {
    auto styles = R"(
      any > (:row and :hover) >> (Icon and :hover) {}
    )";
    auto parser = TokenParser();
    parser.feed(styles);
    auto selector =
      parse_selector(parser, std::make_shared<DefaultSelectorParseStrategy>());
    REQUIRE(selector ==
      DescendantSelector(
        ChildSelector(Any(), AndSelector(Row(), Hover())),
        AndSelector(is_a<Icon>(), Hover())));
  }
}
