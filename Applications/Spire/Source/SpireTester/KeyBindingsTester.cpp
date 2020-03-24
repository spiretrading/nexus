#include <catch.hpp>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include <iostream>
#include <type_traits>
#include <variant>
#include <boost/optional/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <QKeySequence>
#include "Nexus/Definitions/Region.hpp"

using namespace Nexus;
using namespace Spire;
using Action = KeyBindings::Action;
using OrderAction = KeyBindings::OrderAction;
using CancelAction = KeyBindings::CancelAction;

namespace std {
  ostream& operator <<(ostream& os, const Action& action) {
    visit([&] (auto& action) {
      using Type = decay_t<decltype(action)>;
      if constexpr(is_same_v<Type, CancelAction>) {
        os << "(CancelAction: ";
        switch(action) {
          case CancelAction::MOST_RECENT:
            os << "MOST_RECENT";
            break;
          case CancelAction::MOST_RECENT_ASK:
            os << "MOST_RECENT_ASK";
            break;
          case CancelAction::MOST_RECENT_BID:
            os << "MOST_RECENT_BID";
            break;
          case CancelAction::OLDEST:
            os << "OLDEST";
            break;
          case CancelAction::OLDEST_ASK:
            os << "OLDEST_ASK";
            break;
          case CancelAction::OLDEST_BID:
            os << "OLDEST_BID";
            break;
          case CancelAction::ALL:
            os << "ALL";
            break;
          case CancelAction::ALL_ASKS:
            os << "ALL_ASKS";
            break;
          case CancelAction::ALL_BIDS:
            os << "ALL_BIDS";
            break;
          case CancelAction::CLOSEST_ASK:
            os << "CLOSEST_ASK";
            break;
          case CancelAction::CLOSEST_BID:
            os << "CLOSEST_BID";
            break;
          case CancelAction::FURTHEST_ASK:
            os << "CLOSEST_ASK";
            break;
          case CancelAction::FURTHEST_BID:
            os << "CLOSEST_BID";
            break;
        }
        os << ")";
      } else {
        os << "(OrderAction: " << action.m_name << ", " << action.m_type <<
          ", " << action.m_side << ", " << action.m_region.GetName() << ", " <<
          action.m_time_in_force << ", " << action.m_quantity << ", tags: [";
        for(auto& tag : action.m_tags) {
          os << tag.m_name << ", ";
        }
        os << "])";
      }
    }, action);
    return os;
  }
}

namespace {
  void require_no_binding(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence) {
    REQUIRE(bindings.find_binding(region, sequence) == boost::none);
  }

  void require_order_action(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence, const OrderAction& expected) {
    auto action = bindings.find_binding(region, sequence);
    REQUIRE(action != boost::none);
    auto actual = std::get_if<OrderAction>(&(*action));
    REQUIRE(actual != nullptr);
    REQUIRE(expected.m_name == actual->m_name);
    REQUIRE(expected.m_type == actual->m_type);
    REQUIRE(expected.m_side == actual->m_side);
    REQUIRE(expected.m_region == actual->m_region);
    REQUIRE(expected.m_time_in_force == actual->m_time_in_force);
    REQUIRE(expected.m_quantity == actual->m_quantity);
    auto& expected_tags = expected.m_tags;
    auto& actual_tags = actual->m_tags;
    REQUIRE(expected_tags.size() == actual_tags.size());
    for(auto i = std::size_t(0); i < expected_tags.size(); ++i) {
      auto& expected_tag = expected_tags[i];
      auto& actual_tag = actual_tags[i];
      REQUIRE(expected_tag.m_name == actual_tag.m_name);
      std::visit(
        [&] (auto& lhs, auto& rhs) {
          if constexpr(std::is_same_v<decltype(lhs), decltype(rhs)>) {
            REQUIRE(lhs == rhs);
          } else {
            FAIL();
          }
        }, expected_tag.m_value, actual_tag.m_value);
    }
  }
}

void require_cancel_action(const KeyBindings& bindings, const Region& region,
  QKeySequence sequence, const CancelAction& expected) {
  auto action = bindings.find_binding(region, sequence);
  REQUIRE(action != boost::none);
  auto actual = std::get_if<CancelAction>(&(*action));
  REQUIRE(actual != nullptr);
  REQUIRE(expected == *actual);
}

TEST_CASE("test_no_bindings", "[KeyBindings]") {
  auto bindings = KeyBindings();
  REQUIRE(bindings.find_binding(Region(Region::GlobalTag()),
    QKeySequence(Qt::CTRL + Qt::Key_F1)) == boost::none);
  REQUIRE(bindings.find_binding(Region(CountryCode(8)),
    QKeySequence(Qt::SHIFT + Qt::Key_F2)) == boost::none);
  REQUIRE(bindings.find_binding(Region(Security("MSFT", 8)),
    QKeySequence(Qt::Key_F8)) == boost::none);
}

TEST_CASE("test_global_set_and_get", "[KeyBindings]") {
  auto bindings = KeyBindings();
  auto region = Region(Region::GlobalTag());
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, action);
    require_cancel_action(bindings, region, sequence, action);
  }
  require_no_binding(bindings, region, Qt::Key_F1);
  require_no_binding(bindings, region, Qt::SHIFT + Qt::Key_F1);
  require_no_binding(bindings, region, Qt::Key_F2);
  require_no_binding(bindings, region, Qt::Key_F3);
}

TEST_CASE("test_global_override", "[KeyBindings]") {
  auto bindings = KeyBindings();
  auto region = Region(Region::GlobalTag());
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, action);
    require_cancel_action(bindings, region, sequence, action);
  }
}

TEST_CASE("test_local_set_and_get", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action3", OrderType::PEGGED, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
  }
}

TEST_CASE("test_distinct_regions", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_no_binding(bindings, Region(CountryCode(4)), sequence);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_no_binding(bindings, Region(Security("GOOG", CountryCode(4))),
      sequence);
  }
}

TEST_CASE("test_wider_regions", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_no_binding(bindings, Region(CountryCode(4)), sequence);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
  {
    auto region = Region(Security("MSFT", MarketCode("NSDQ"), CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action3", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(50), {} };
    bindings.set_binding(sequence, action);
    require_no_binding(bindings, Region(CountryCode(4)), sequence);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action4", OrderType::PEGGED, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, action);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
}

TEST_CASE("test_narrower_regions", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, Region(Security("MSFT", CountryCode(8))),
      sequence, action);
    require_order_action(bindings, Region(Security("GOOG", CountryCode(8))),
      sequence, action);
    require_order_action(bindings, Region(Security("MSFT", MarketCode("NSDQ"),
      CountryCode(8))), sequence, action);
    require_order_action(bindings, Region(Security("GOOG", MarketCode("NYSE"),
      CountryCode(8))), sequence, action);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, Region(Security("MSFT", MarketCode("NSDQ"),
      CountryCode(4))), sequence, action);
    require_order_action(bindings, Region(Security("MSFT", MarketCode("NYSE"),
      CountryCode(4))), sequence, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action3", OrderType::PEGGED, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, Region(CountryCode(8)),
      sequence, action);
    require_order_action(bindings, Region(Security("GOOG", CountryCode(8))),
      sequence, action);
    require_order_action(bindings, Region(Security("MSFT", MarketCode("NSDQ"),
      CountryCode(8))), sequence, action);
  }
}

TEST_CASE("test_different_actions_in_distinct_regions", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto region1 = Region(CountryCode(8)); 
    auto action1 = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region1, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action1);
    auto region2 = Region(CountryCode(4)); 
    auto action2 = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region2, TimeInForce(TimeInForce::Type::DAY), Quantity(8), {} };
    bindings.set_binding(sequence, action2);
    require_order_action(bindings, region1, sequence, action1);
    require_order_action(bindings, region2, sequence, action2);
  }
  {
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto region1 = Region(Security("MSFT", CountryCode(4)));
    auto action1 = OrderAction{ "action3", OrderType::LIMIT, Side::ASK,
      region1, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action1);
    auto region2 = Region(Security("GOOG", CountryCode(4)));
    auto action2 = OrderAction{ "action4", OrderType::LIMIT, Side::BID,
      region2, TimeInForce(TimeInForce::Type::DAY), Quantity(1), {} };
    bindings.set_binding(sequence, action2);
    require_order_action(bindings, region1, sequence, action1);
    require_order_action(bindings, region2, sequence, action2);
  }
}

TEST_CASE("test_different_actions_in_region_hierarchy", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto region1 = Region(CountryCode(8));
    auto action1 = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region1, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action1);
    auto region2 = Region(CountryCode(4));
    auto action2 = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region2, TimeInForce(TimeInForce::Type::DAY), Quantity(8), {} };
    bindings.set_binding(sequence, action2);
    auto region3 = Region(Security("MSFT", CountryCode(8)));
    auto action3 = OrderAction{ "action3", OrderType::PEGGED, Side::BID,
      region3, TimeInForce(TimeInForce::Type::DAY), Quantity(15), {} };
    bindings.set_binding(sequence, action3);
    auto region4 = Region(Security("GOOG", CountryCode(8)));
    auto action4 = OrderAction{ "action4", OrderType::PEGGED, Side::ASK,
      region4, TimeInForce(TimeInForce::Type::DAY), Quantity(25), {} };
    bindings.set_binding(sequence, action4);
    auto region5 = Region(Security("MSFT", CountryCode(4)));
    auto action5 = OrderAction{ "action5", OrderType::MARKET, Side::ASK,
      region5, TimeInForce(TimeInForce::Type::DAY), Quantity(35), {} };
    bindings.set_binding(sequence, action5);
    auto region6 = Region(Security("MSFT", CountryCode(1)));
    auto action6 = OrderAction{ "action5", OrderType::MARKET, Side::ASK,
      region6, TimeInForce(TimeInForce::Type::DAY), Quantity(45), {} };
    bindings.set_binding(sequence, action6);
    require_order_action(bindings, region1, sequence, action1);
    require_order_action(bindings, region2, sequence, action2);
    require_order_action(bindings, region3, sequence, action3);
    require_order_action(bindings, region4, sequence, action4);
    require_order_action(bindings, region5, sequence, action5);
    require_order_action(bindings, region6, sequence, action6);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
    require_no_binding(bindings, Region(CountryCode(1)), sequence);
    require_order_action(bindings, Region(Security("TSLA", CountryCode(8))),
      sequence, action1);
    require_order_action(bindings, Region(Security("GOOG", CountryCode(4))),
      sequence, action2);
  }
  {
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto region1 = Region(Region::GlobalTag());
    auto action1 = OrderAction{ "action1", OrderType::LIMIT, Side::ASK,
      region1, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action1);
    auto region2 = Region(CountryCode(4));
    auto action2 = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region2, TimeInForce(TimeInForce::Type::DAY), Quantity(8), {} };
    bindings.set_binding(sequence, action2);
    auto region3 = Region(Security("MSFT", CountryCode(4)));
    auto action3 = OrderAction{ "action3", OrderType::PEGGED, Side::BID,
      region3, TimeInForce(TimeInForce::Type::DAY), Quantity(15), {} };
    bindings.set_binding(sequence, action3);
    auto region4 = Region(Security("GOOG", CountryCode(8)));
    auto action4 = OrderAction{ "action4", OrderType::PEGGED, Side::BID,
      region4, TimeInForce(TimeInForce::Type::DAY), Quantity(15), {} };
    bindings.set_binding(sequence, action4);
    require_order_action(bindings, region1, sequence, action1);
    require_order_action(bindings, region2, sequence, action2);
    require_order_action(bindings, region3, sequence, action3);
    require_order_action(bindings, region4, sequence, action4);
    require_order_action(bindings, Region(CountryCode(1)), sequence, action1);
    require_order_action(bindings, Region(CountryCode(8)), sequence, action1);
    require_order_action(bindings, Region(Security("GOOG", CountryCode(4))),
      sequence, action2);
    require_order_action(bindings, Region(Security("TSLA", CountryCode(8))),
      sequence, action1);
    require_order_action(bindings, Region(Security("MSFT", CountryCode(1))),
      sequence, action1);
  }
}

TEST_CASE("test_reset", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, action);
    require_cancel_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action3", OrderType::MARKET, Side::BID,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action4", OrderType::LIMIT, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action5", OrderType::PEGGED, Side::ASK,
      region, TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
}
