#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Spire/KeyBindings/KeyBindings.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  void require_no_binding(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence) {
    REQUIRE(bindings.find(region, sequence) == boost::none);
  }

  bool are_same(const KeyBindings::OrderAction& actual,
      const KeyBindings::OrderAction& expected) {
    if(expected.get_name() != actual.get_name()) {
      return false;
    }
    auto expected_tags = expected.get_tags();
    auto actual_tags = actual.get_tags();
    REQUIRE(expected_tags.size() == actual_tags.size());
    for(auto i = std::size_t(0); i < expected_tags.size(); ++i) {
      auto& expected_tag = expected_tags[i];
      auto& actual_tag = actual_tags[i];
      if(expected_tag.get_key() != actual_tag.get_key()) {
        return false;
      }
      auto same = std::visit(
        [&] (auto& lhs, auto& rhs) {
          if constexpr(std::is_same_v<decltype(lhs), decltype(rhs)>) {
            return lhs == rhs;
          } else {
            return false;
          }
        }, expected_tag.get_value(), actual_tag.get_value());
      if(!same) {
        return false;
      }
    }
    return true;
  }

  void require_order_action(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence, const KeyBindings::OrderAction& expected) {
    auto action = bindings.find(region, sequence);
    REQUIRE(action != boost::none);
    auto actual = std::get_if<KeyBindings::OrderAction>(&(*action));
    REQUIRE(actual != nullptr);
    REQUIRE(are_same(*actual, expected));
  }

  void require_cancel_action(const KeyBindings& bindings, const Region& region,
      QKeySequence sequence, const KeyBindings::CancelAction& expected) {
    auto action = bindings.find(region, sequence);
    REQUIRE(action != boost::none);
    auto actual = std::get_if<KeyBindings::CancelAction>(&(*action));
    REQUIRE(actual != nullptr);
    REQUIRE(expected == *actual);
  }

  void require_same_lists(
      const std::vector<KeyBindings::OrderActionBinding>& actual,
      const std::vector<KeyBindings::OrderActionBinding>& expected) {
    REQUIRE(actual.size() == expected.size());
    for(auto& lhs : actual) {
      REQUIRE(std::find_if(expected.begin(), expected.end(),
        [&] (auto& rhs) {
          return lhs.m_region == rhs.m_region &&
            are_same(lhs.m_action, rhs.m_action);
        }) != expected.end());
    }
  }

  void require_same_lists(
      const std::vector<KeyBindings::CancelActionBinding>& actual,
      const std::vector<KeyBindings::CancelActionBinding>& expected) {
    REQUIRE(actual.size() == expected.size());
    for(auto& binding : actual) {
      REQUIRE(std::find(expected.begin(), expected.end(), binding) !=
        expected.end());
    }
  }

  std::pair<std::vector<KeyBindings::OrderActionBinding>,
      std::vector<KeyBindings::CancelActionBinding>> split_bindings_list(
      const std::vector<KeyBindings::ActionBinding>& list) {
    auto order_list = std::vector<KeyBindings::OrderActionBinding>();
    auto cancel_list = std::vector<KeyBindings::CancelActionBinding>();
    for(auto& binding : list) {
      std::visit([&] (auto& action) {
        using Type = std::decay_t<decltype(action)>;
        if constexpr(std::is_same_v<Type, KeyBindings::OrderAction>) {
          order_list.push_back({binding.m_sequence, binding.m_region, action});
        } else {
          cancel_list.push_back({binding.m_sequence, binding.m_region, action});
        }
      }, binding.m_action);
    }
    return std::make_pair(std::move(order_list), std::move(cancel_list));
  }

  void require_same_lists(
      const std::vector<KeyBindings::ActionBinding>& actual,
      const std::vector<KeyBindings::ActionBinding>& expected) {
    REQUIRE(actual.size() == expected.size());
    auto [actual_order_list, actual_cancel_list] = split_bindings_list(actual);
    auto [expected_order_list, expected_cancel_list] =
      split_bindings_list(expected);
    require_same_lists(actual_order_list, expected_order_list);
    require_same_lists(actual_cancel_list, expected_cancel_list);
  }

  auto create_order_action(std::string name, OrderType type, Side side,
      TimeInForce time_in_force, Quantity quantity) {
    return KeyBindings::OrderAction(std::move(name), {KeyBindings::Tag(
      40, type), KeyBindings::Tag(54, side), KeyBindings::Tag(59,
      time_in_force), KeyBindings::Tag(38, quantity)});
  }
}

TEST_SUITE("KeyBindings") {
  TEST_CASE("no_bindings") {
    auto bindings = KeyBindings();
    REQUIRE(bindings.find(Region::Global(),
      QKeySequence(Qt::CTRL + Qt::Key_F1)) == boost::none);
    REQUIRE(bindings.find(Region(CountryCode(8)),
      QKeySequence(Qt::SHIFT + Qt::Key_F2)) == boost::none);
    REQUIRE(bindings.find(Region(Security("MSFT", DefaultMarkets::NYSE(),
      DefaultCountries::US())), QKeySequence(Qt::Key_F8)) == boost::none);
  }

  TEST_CASE("global_set_and_get") {
    auto bindings = KeyBindings();
    auto region = Region::Global();
    {
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
    {
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
    {
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = KeyBindings::CancelAction::CLOSEST_ASK;
      bindings.set(sequence, region, action);
      require_cancel_action(bindings, region, sequence, action);
    }
    require_no_binding(bindings, region, Qt::Key_F1);
    require_no_binding(bindings, region, Qt::SHIFT + Qt::Key_F1);
    require_no_binding(bindings, region, Qt::Key_F2);
    require_no_binding(bindings, region, Qt::Key_F3);
  }

  TEST_CASE("global_override") {
    auto bindings = KeyBindings();
    auto region = Region::Global();
    {
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
    {
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
    {
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = KeyBindings::CancelAction::CLOSEST_ASK;
      bindings.set(sequence, region, action);
      require_cancel_action(bindings, region, sequence, action);
    }
  }

  TEST_CASE("local_set_and_get") {
    auto bindings = KeyBindings();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
    {
      auto region = Region(Security("MSFT", CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
    {
      auto region = Region("named_region");
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = create_order_action("action3", OrderType::PEGGED,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 100);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
    }
  }

  TEST_CASE("distinct_regions") {
    auto bindings = KeyBindings();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_no_binding(bindings, Region(CountryCode(4)), sequence);
    }
    {
      auto region = Region(Security("MSFT", CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_no_binding(bindings, Region(Security("GOOG", CountryCode(4))),
        sequence);
    }
  }

  TEST_CASE("wider_regions") {
    auto bindings = KeyBindings();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_no_binding(bindings, Region::Global(), sequence);
    }
    {
      auto region = Region(Security("MSFT", CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_no_binding(bindings, Region(CountryCode(4)), sequence);
      require_no_binding(bindings, Region::Global(), sequence);
    }
    {
      auto region = Region(Security("MSFT", MarketCode("NSDQ"), CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action3", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 50);
      bindings.set(sequence, region, action);
      require_no_binding(bindings, Region(CountryCode(4)), sequence);
      require_no_binding(bindings, Region::Global(), sequence);
    }
    {
      auto region = Region("named_region");
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = create_order_action("action4", OrderType::PEGGED,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 100);
      bindings.set(sequence, region, action);
      require_no_binding(bindings, Region::Global(), sequence);
    }
  }

  TEST_CASE("narrower_regions") {
    auto bindings = KeyBindings();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
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
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_order_action(bindings, Region(Security("MSFT", MarketCode("NSDQ"),
        CountryCode(4))), sequence, action);
      require_order_action(bindings, Region(Security("MSFT", MarketCode("NYSE"),
        CountryCode(4))), sequence, action);
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = create_order_action("action3", OrderType::PEGGED,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 100);
      bindings.set(sequence, region, action);
      require_order_action(bindings, Region(CountryCode(8)),
        sequence, action);
      require_order_action(bindings, Region(Security("GOOG", CountryCode(8))),
        sequence, action);
      require_order_action(bindings, Region(Security("MSFT", MarketCode("NSDQ"),
        CountryCode(8))), sequence, action);
    }
  }

  TEST_CASE("different_actions_in_distinct_regions") {
    auto bindings = KeyBindings();
    {
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto region1 = Region(CountryCode(8)); 
      auto action1 = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region1, action1);
      auto region2 = Region(CountryCode(4)); 
      auto action2 = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 8);
      bindings.set(sequence, region2, action2);
      require_order_action(bindings, region1, sequence, action1);
      require_order_action(bindings, region2, sequence, action2);
    }
    {
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto region1 = Region(Security("MSFT", CountryCode(4)));
      auto action1 = create_order_action("action3", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region1, action1);
      auto region2 = Region(Security("GOOG", CountryCode(4)));
      auto action2 = create_order_action("action4", OrderType::LIMIT,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 1);
      bindings.set(sequence, region2, action2);
      require_order_action(bindings, region1, sequence, action1);
      require_order_action(bindings, region2, sequence, action2);
    }
  }

  TEST_CASE("different_actions_in_region_hierarchy") {
    auto bindings = KeyBindings();
    {
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto region1 = Region(CountryCode(8));
      auto action1 = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region1, action1);
      auto region2 = Region(CountryCode(4));
      auto action2 = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 8);
      bindings.set(sequence, region2, action2);
      auto region3 = Region(Security("MSFT", CountryCode(8)));
      auto action3 = create_order_action("action3", OrderType::PEGGED,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 15);
      bindings.set(sequence, region3, action3);
      auto region4 = Region(Security("GOOG", CountryCode(8)));
      auto action4 = create_order_action("action4", OrderType::PEGGED,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 25);
      bindings.set(sequence, region4, action4);
      auto region5 = Region(Security("MSFT", CountryCode(4)));
      auto action5 = create_order_action("action5", OrderType::MARKET,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 35);
      bindings.set(sequence, region5, action5);
      auto region6 = Region(Security("MSFT", CountryCode(1)));
      auto action6 = create_order_action("action5", OrderType::MARKET,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 45);
      bindings.set(sequence, region6, action6);
      require_order_action(bindings, region1, sequence, action1);
      require_order_action(bindings, region2, sequence, action2);
      require_order_action(bindings, region3, sequence, action3);
      require_order_action(bindings, region4, sequence, action4);
      require_order_action(bindings, region5, sequence, action5);
      require_order_action(bindings, region6, sequence, action6);
      require_no_binding(bindings, Region::Global(), sequence);
      require_no_binding(bindings, Region(CountryCode(1)), sequence);
      require_order_action(bindings, Region(Security("TSLA", CountryCode(8))),
        sequence, action1);
      require_order_action(bindings, Region(Security("GOOG", CountryCode(4))),
        sequence, action2);
    }
    {
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto region1 = Region::Global();
      auto action1 = create_order_action("action1", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region1, action1);
      auto region2 = Region(CountryCode(4));
      auto action2 = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 8);
      bindings.set(sequence, region2, action2);
      auto region3 = Region(Security("MSFT", CountryCode(4)));
      auto action3 = create_order_action("action3", OrderType::PEGGED,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 15);
      bindings.set(sequence, region3, action3);
      auto region4 = Region(Security("GOOG", CountryCode(8)));
      auto action4 = create_order_action("action4", OrderType::PEGGED,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 15);
      bindings.set(sequence, region4, action4);
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

  TEST_CASE("reset") {
    auto bindings = KeyBindings();
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
      bindings.reset(region, sequence);
      require_no_binding(bindings, region, sequence);
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
      bindings.reset(region, sequence);
      require_no_binding(bindings, region, sequence);
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = KeyBindings::CancelAction::CLOSEST_ASK;
      bindings.set(sequence, region, action);
      require_cancel_action(bindings, region, sequence, action);
      bindings.reset(region, sequence);
      require_no_binding(bindings, region, sequence);
    }
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action3", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
      bindings.reset(region, sequence);
      require_no_binding(bindings, region, sequence);
    }
    {
      auto region = Region(Security("MSFT", CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action4", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
      bindings.reset(region, sequence);
      require_no_binding(bindings, region, sequence);
    }
    {
      auto region = Region("named_region");
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = create_order_action("action5", OrderType::PEGGED,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 100);
      bindings.set(sequence, region, action);
      require_order_action(bindings, region, sequence, action);
      bindings.reset(region, sequence);
      require_no_binding(bindings, region, sequence);
    }
  }

  TEST_CASE("order_bindings_list") {
    auto bindings = KeyBindings();
    auto expected = std::vector<KeyBindings::OrderActionBinding>();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
    }
    {
      auto region = Region(Security("MSFT", CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region(CountryCode(4));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action3", OrderType::MARKET,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region("named_region");
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = create_order_action("action4", OrderType::PEGGED,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 100);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action5", OrderType::LIMIT,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = KeyBindings::CancelAction::CLOSEST_ASK;
      bindings.set(sequence, region, action);
    }
    require_same_lists(bindings.build_order_bindings(), expected);
  }

  TEST_CASE("cancel_bindings_list") {
    auto bindings = KeyBindings();
    auto expected = std::vector<KeyBindings::CancelActionBinding>();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = KeyBindings::CancelAction::CLOSEST_ASK;
      bindings.set(sequence, region, action);
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = KeyBindings::CancelAction::CLOSEST_BID;
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F8);
      auto action = KeyBindings::CancelAction::MOST_RECENT;
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    require_same_lists(bindings.build_cancel_bindings(), expected);
  }

  TEST_CASE("action_bindings_list") {
    auto bindings = KeyBindings();
    auto expected = std::vector<KeyBindings::ActionBinding>();
    {
      auto region = Region(CountryCode(8));
      auto sequence = QKeySequence(Qt::CTRL + Qt::Key_F1);
      auto action = create_order_action("action1", OrderType::MARKET,
        Side::BID, TimeInForce(TimeInForce::Type::DAY), 5);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region(Security("MSFT", CountryCode(4)));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action2", OrderType::LIMIT,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region(CountryCode(4));
      auto sequence = QKeySequence(Qt::SHIFT + Qt::Key_F3);
      auto action = create_order_action("action3", OrderType::MARKET,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 10);
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F4);
      auto action = create_order_action("action4", OrderType::MARKET,
        Side::ASK, TimeInForce(TimeInForce::Type::DAY), 100);
      bindings.set(sequence, region, action);
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F8);
      auto action = KeyBindings::CancelAction::CLOSEST_ASK;
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    {
      auto region = Region::Global();
      auto sequence = QKeySequence(Qt::Key_F4);
      auto action = KeyBindings::CancelAction::MOST_RECENT;
      bindings.set(sequence, region, action);
      expected.push_back({sequence, region, action});
    }
    require_same_lists(bindings.build_action_bindings(), expected);
  }
}
