#include <deque>
#include <doctest/doctest.h>
#include "Spire/BookView/PreviewOrderDisplayValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  using OrderVisibility = BookViewHighlightProperties::OrderVisibility;

  const auto TEST_FIELDS = make_limit_order_fields(
    parse_ticker("ABC.TSX"), Side::BID, 100, Money::CENT);

  void set_visibility(
      BookViewPropertiesModel& properties, OrderVisibility visibility) {
    auto update = properties.get();
    update.m_highlight_properties.m_order_visibility = visibility;
    properties.set(update);
  }
}

TEST_SUITE("PreviewOrderDisplayValueModel") {
  TEST_CASE("constructor_none_hidden") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    set_visibility(*properties, OrderVisibility::HIDDEN);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    auto updates = std::deque<optional<OrderFields>>();
    display.connect_update_signal([&] (const auto& update) {
      updates.push_back(update);
    });
    REQUIRE((display.get() == none));
    SUBCASE("transition_visible") {
      set_visibility(*properties, OrderVisibility::VISIBLE);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_highlighted") {
      set_visibility(*properties, OrderVisibility::HIGHLIGHTED);
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
      set_visibility(*properties, OrderVisibility::VISIBLE);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
    SUBCASE("transition_visible_some") {
      set_visibility(*properties, OrderVisibility::VISIBLE);
      preview->set(TEST_FIELDS);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
  }

  TEST_CASE("constructor_none_visible") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    set_visibility(*properties, OrderVisibility::VISIBLE);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    REQUIRE((display.get() == none));
    auto updates = std::deque<optional<OrderFields>>();
    display.connect_update_signal([&] (const auto& update) {
      updates.push_back(update);
    });
    SUBCASE("transition_hidden") {
      set_visibility(*properties, OrderVisibility::HIDDEN);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
    SUBCASE("transition_highlighted") {
      set_visibility(*properties, OrderVisibility::HIGHLIGHTED);
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
      set_visibility(*properties, OrderVisibility::HIDDEN);
      REQUIRE((display.get() == none));
      REQUIRE(updates.size() == 2);
    }
    SUBCASE("transition_hidden_some") {
      set_visibility(*properties, OrderVisibility::HIDDEN);
      preview->set(TEST_FIELDS);
      REQUIRE((display.get() == none));
      REQUIRE(updates.empty());
    }
  }

  TEST_CASE("constructor_none_highlighted") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    set_visibility(*properties, OrderVisibility::HIGHLIGHTED);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    REQUIRE((display.get() == none));
  }

  TEST_CASE("constructor_some_hidden") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(TEST_FIELDS);
    auto properties = std::make_shared<LocalBookViewPropertiesModel>();
    set_visibility(*properties, OrderVisibility::HIDDEN);
    auto display = PreviewOrderDisplayValueModel(preview, properties);
    auto updates = std::deque<optional<OrderFields>>();
    display.connect_update_signal([&] (const auto& update) {
      updates.push_back(update);
    });
    REQUIRE((display.get() == none));
    SUBCASE("transition_visible") {
      set_visibility(*properties, OrderVisibility::VISIBLE);
      REQUIRE((display.get() == TEST_FIELDS));
      REQUIRE(updates.size() == 1);
    }
  }

  TEST_CASE("constructor_some_displayed") {
    for(auto visibility :
        {OrderVisibility::VISIBLE, OrderVisibility::HIGHLIGHTED}) {
      auto preview =
        std::make_shared<LocalValueModel<optional<OrderFields>>>();
      preview->set(TEST_FIELDS);
      auto properties = std::make_shared<LocalBookViewPropertiesModel>();
      set_visibility(*properties, visibility);
      auto display = PreviewOrderDisplayValueModel(preview, properties);
      REQUIRE((display.get() == TEST_FIELDS));
    }
  }
}
