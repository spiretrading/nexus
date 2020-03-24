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
using ActionBindingsList = KeyBindings::ActionBindingsList;
using OrderBindingsList = KeyBindings::OrderBindingsList;
using CancelBindingsList = KeyBindings::CancelBindingsList;

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
          ", " << action.m_side << ", " << action.m_time_in_force << ", " <<
          action.m_quantity << ", tags: [";
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

  bool are_same(const OrderAction& actual, const OrderAction& expected) {
    if(expected.m_name != actual.m_name ||
        expected.m_type != actual.m_type ||
        expected.m_side != actual.m_side ||
        expected.m_time_in_force != actual.m_time_in_force ||
        expected.m_quantity != actual.m_quantity) {
      return false;
    }
    auto& expected_tags = expected.m_tags;
    auto& actual_tags = actual.m_tags;
    REQUIRE(expected_tags.size() == actual_tags.size());
    for(auto i = std::size_t(0); i < expected_tags.size(); ++i) {
      auto& expected_tag = expected_tags[i];
      auto& actual_tag = actual_tags[i];
      if(expected_tag.m_name != actual_tag.m_name) {
        return false;
      }
      auto same = std::visit(
        [&] (auto& lhs, auto& rhs) {
          if constexpr(std::is_same_v<decltype(lhs), decltype(rhs)>) {
            return lhs == rhs;
          } else {
            return false;
          }
        }, expected_tag.m_value, actual_tag.m_value);
      if(!same) {
        return false;
      }
    }
    return true;
  }

  void require_order_action(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence, const OrderAction& expected) {
    auto action = bindings.find_binding(region, sequence);
    REQUIRE(action != boost::none);
    auto actual = std::get_if<OrderAction>(&(*action));
    REQUIRE(actual != nullptr);
    REQUIRE(are_same(*actual, expected));
  }

  void require_cancel_action(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence, const CancelAction& expected) {
    auto action = bindings.find_binding(region, sequence);
    REQUIRE(action != boost::none);
    auto actual = std::get_if<CancelAction>(&(*action));
    REQUIRE(actual != nullptr);
    REQUIRE(expected == *actual);
  }

  void require_same_lists(const OrderBindingsList& actual,
      const OrderBindingsList& expected) {
    REQUIRE(actual.size() == expected.size());
    for(auto& lhs : actual) {
      REQUIRE(std::find_if(expected.begin(), expected.end(),
        [&] (auto& rhs) {
          return lhs.first == rhs.first && are_same(lhs.second, rhs.second);
        }) != expected.end());
    }
  }

  void require_same_lists(const CancelBindingsList& actual,
      const CancelBindingsList& expected) {
    REQUIRE(actual.size() == expected.size());
    for(auto& binding : actual) {
      REQUIRE(std::find(expected.begin(), expected.end(), binding) !=
        expected.end());
    }
  }

  std::pair<OrderBindingsList, CancelBindingsList> split_bindings_list(
      const ActionBindingsList& list) {
    auto order_list = OrderBindingsList();
    auto cancel_list = CancelBindingsList();
    for(auto& binding : list) {
      std::visit([&] (auto& action) {
        using Type = std::decay_t<decltype(action)>;
        if constexpr(std::is_same_v<Type, OrderAction>) {
          order_list.emplace_back(binding.first, action);
        } else {
          cancel_list.emplace_back(binding.first, action);
        }
      }, binding.second);
    }
    return std::make_pair(std::move(order_list), std::move(cancel_list));
  }

  void require_same_lists(const ActionBindingsList& actual,
    const ActionBindingsList& expected) {
    REQUIRE(actual.size() == expected.size());
    auto [actual_order_list, actual_cancel_list] = split_bindings_list(actual);
    auto [expected_order_list, expected_cancel_list] =
      split_bindings_list(expected);
    require_same_lists(actual_order_list, expected_order_list);
    require_same_lists(actual_cancel_list, expected_cancel_list);
  }
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, region, action);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, region, action);
    require_cancel_action(bindings, region, sequence, action);
  }
}

TEST_CASE("test_local_set_and_get", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action3", OrderType::PEGGED, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
  }
}

TEST_CASE("test_distinct_regions", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_no_binding(bindings, Region(CountryCode(4)), sequence);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_no_binding(bindings, Region(CountryCode(4)), sequence);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
  {
    auto region = Region(Security("MSFT", MarketCode("NSDQ"), CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action3", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(50), {} };
    bindings.set_binding(sequence, region, action);
    require_no_binding(bindings, Region(CountryCode(4)), sequence);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action4", OrderType::PEGGED, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, region, action);
    require_no_binding(bindings, Region(Region::GlobalTag()), sequence);
  }
}

TEST_CASE("test_narrower_regions", "[KeyBindings]") {
  auto bindings = KeyBindings();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, Region(Security("MSFT", MarketCode("NSDQ"),
      CountryCode(4))), sequence, action);
    require_order_action(bindings, Region(Security("MSFT", MarketCode("NYSE"),
      CountryCode(4))), sequence, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action3", OrderType::PEGGED, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, region, action);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region1, action1);
    auto region2 = Region(CountryCode(4)); 
    auto action2 = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(8), {} };
    bindings.set_binding(sequence, region2, action2);
    require_order_action(bindings, region1, sequence, action1);
    require_order_action(bindings, region2, sequence, action2);
  }
  {
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto region1 = Region(Security("MSFT", CountryCode(4)));
    auto action1 = OrderAction{ "action3", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region1, action1);
    auto region2 = Region(Security("GOOG", CountryCode(4)));
    auto action2 = OrderAction{ "action4", OrderType::LIMIT, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(1), {} };
    bindings.set_binding(sequence, region2, action2);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region1, action1);
    auto region2 = Region(CountryCode(4));
    auto action2 = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(8), {} };
    bindings.set_binding(sequence, region2, action2);
    auto region3 = Region(Security("MSFT", CountryCode(8)));
    auto action3 = OrderAction{ "action3", OrderType::PEGGED, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(15), {} };
    bindings.set_binding(sequence, region3, action3);
    auto region4 = Region(Security("GOOG", CountryCode(8)));
    auto action4 = OrderAction{ "action4", OrderType::PEGGED, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(25), {} };
    bindings.set_binding(sequence, region4, action4);
    auto region5 = Region(Security("MSFT", CountryCode(4)));
    auto action5 = OrderAction{ "action5", OrderType::MARKET, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(35), {} };
    bindings.set_binding(sequence, region5, action5);
    auto region6 = Region(Security("MSFT", CountryCode(1)));
    auto action6 = OrderAction{ "action5", OrderType::MARKET, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(45), {} };
    bindings.set_binding(sequence, region6, action6);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region1, action1);
    auto region2 = Region(CountryCode(4));
    auto action2 = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(8), {} };
    bindings.set_binding(sequence, region2, action2);
    auto region3 = Region(Security("MSFT", CountryCode(4)));
    auto action3 = OrderAction{ "action3", OrderType::PEGGED, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(15), {} };
    bindings.set_binding(sequence, region3, action3);
    auto region4 = Region(Security("GOOG", CountryCode(8)));
    auto action4 = OrderAction{ "action4", OrderType::PEGGED, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(15), {} };
    bindings.set_binding(sequence, region4, action4);
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
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, region, action);
    require_cancel_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action3", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action4", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action5", OrderType::PEGGED, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, region, action);
    require_order_action(bindings, region, sequence, action);
    bindings.reset_binding(region, sequence);
    require_no_binding(bindings, region, sequence);
  }
}

TEST_CASE("test_order_bindings_list", "[KeyBindings]") {
  auto bindings = KeyBindings();
  auto expected = OrderBindingsList();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(CountryCode(4));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action3", OrderType::MARKET, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region("named_region");
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = OrderAction{ "action4", OrderType::PEGGED, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action5", OrderType::LIMIT, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, region, action);
  }
  require_same_lists(bindings.build_order_bindings_list(), expected);
}

TEST_CASE("test_cancel_bindings_list", "[KeyBindings]") {
  auto bindings = KeyBindings();
  auto expected = CancelBindingsList();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, region, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_BID;
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F8);
    auto action = CancelAction::MOST_RECENT;
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  require_same_lists(bindings.build_cancel_bindings_list(), expected);
}

TEST_CASE("test_action_bindings_list", "[KeyBindings]") {
  auto bindings = KeyBindings();
  auto expected = ActionBindingsList();
  {
    auto region = Region(CountryCode(8));
    auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
    auto action = OrderAction{ "action1", OrderType::MARKET, Side::BID,
      TimeInForce(TimeInForce::Type::DAY), Quantity(5), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(Security("MSFT", CountryCode(4)));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action2", OrderType::LIMIT, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(CountryCode(4));
    auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    auto action = OrderAction{ "action3", OrderType::MARKET, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(10), {} };
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F4);
    auto action = OrderAction{ "action4", OrderType::MARKET, Side::ASK,
      TimeInForce(TimeInForce::Type::DAY), Quantity(100), {} };
    bindings.set_binding(sequence, region, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F8);
    auto action = CancelAction::CLOSEST_ASK;
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  {
    auto region = Region(Region::GlobalTag());
    auto sequence = QKeySequence(Qt::Key_F4);
    auto action = CancelAction::MOST_RECENT;
    bindings.set_binding(sequence, region, action);
    expected.emplace_back(sequence, action);
  }
  require_same_lists(bindings.build_action_bindings_list(), expected);
}
