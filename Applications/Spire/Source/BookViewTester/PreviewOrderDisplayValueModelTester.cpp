#include <doctest/doctest.h>
#include <deque>
#include "Spire/BookView/PreviewOrderDisplayValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  auto TEST_FIELDS = make_limit_order_fields(
    parse_security("ABC.TSX"), Side::BID, 100, Money::CENT);
}

TEST_SUITE("PreviewOrderDisplayValueModel") {
  TEST_CASE("constructor_none_hidden") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    auto updated_properties = properties->get();
    updated_properties.m_highlight_properties.m_order_visibility =
      BookViewHighlightProperties::OrderVisibility::HIDDEN;
    properties->set(updated_properties);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    auto updates = std::deque<optional<OrderFields>>();
    display.connect_update_signal([&] (const auto& update) {
      updates.push_back(update);
    });
    REQUIRE((display.get() == none));
    SUBCASE("transition_visible") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::VISIBLE;
      properties->set(updated_properties);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_highlighted") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED;
      properties->set(updated_properties);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_some") {
      preview->set(TEST_FIELDS);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_some_visible") {
      preview->set(TEST_FIELDS);
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::VISIBLE;
      properties->set(updated_properties);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
    SUBCASE("transition_visible_some") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::VISIBLE;
      properties->set(updated_properties);
      preview->set(TEST_FIELDS);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
  }
  TEST_CASE("constructor_none_visible") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    auto updated_properties = properties->get();
    updated_properties.m_highlight_properties.m_order_visibility =
      BookViewHighlightProperties::OrderVisibility::VISIBLE;
    properties->set(updated_properties);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    REQUIRE((display.get() == none));
    auto updates = std::deque<optional<OrderFields>>();
    display.connect_update_signal([&] (const auto& update) {
      updates.push_back(update);
    });
    SUBCASE("transition_hidden") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::HIDDEN;
      properties->set(updated_properties);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_highlighted") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED;
      properties->set(updated_properties);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_some") {
      preview->set(TEST_FIELDS);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
    SUBCASE("transition_some_hidden") {
      preview->set(TEST_FIELDS);
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::HIDDEN;
      properties->set(updated_properties);
      REQUIRE((display.get() == none));
      REQUIRE(updates.size() == 2);
    }
    SUBCASE("transition_hidden_some") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::HIDDEN;
      properties->set(updated_properties);
      preview->set(TEST_FIELDS);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
  }
  TEST_CASE("constructor_none_highlighted") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    auto updated_properties = properties->get();
    updated_properties.m_highlight_properties.m_order_visibility =
      BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED;
    properties->set(updated_properties);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    REQUIRE((display.get() == none));
  }
  TEST_CASE("constructor_some_hidden") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(TEST_FIELDS);
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    auto updated_properties = properties->get();
    updated_properties.m_highlight_properties.m_order_visibility =
      BookViewHighlightProperties::OrderVisibility::HIDDEN;
    properties->set(updated_properties);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    auto updates = std::deque<optional<OrderFields>>();
    display.connect_update_signal([&] (const auto& update) {
      updates.push_back(update);
    });
    REQUIRE((display.get() == none));
    SUBCASE("transition_visible") {
      auto updated_properties = properties->get();
      updated_properties.m_highlight_properties.m_order_visibility =
        BookViewHighlightProperties::OrderVisibility::VISIBLE;
      properties->set(updated_properties);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
  }
  TEST_CASE("constructor_some_visible") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(TEST_FIELDS);
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    auto updated_properties = properties->get();
    updated_properties.m_highlight_properties.m_order_visibility =
      BookViewHighlightProperties::OrderVisibility::VISIBLE;
    properties->set(updated_properties);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    REQUIRE((display.get() == TEST_FIELDS));
  }
  TEST_CASE("constructor_some_highlighted") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(TEST_FIELDS);
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    auto updated_properties = properties->get();
    updated_properties.m_highlight_properties.m_order_visibility =
      BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED;
    properties->set(updated_properties);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    REQUIRE((display.get() == TEST_FIELDS));
  }
}
