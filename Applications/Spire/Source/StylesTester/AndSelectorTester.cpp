#include <deque>
#include <doctest/doctest.h>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

namespace {
  struct DisconnectingSelector {
    bool operator ==(const DisconnectingSelector&) const = default;
  };

  SelectConnection select(const DisconnectingSelector& selector,
      const Stylist& base, const SelectionUpdateSignal& on_update) {
    struct Executor {
      const Stylist* m_base;
      SelectionUpdateSignal m_on_update;
      boost::signals2::scoped_connection m_connection;
      bool m_is_connected;

      Executor(const Stylist& base, const SelectionUpdateSignal& on_update)
        : m_base(&base),
          m_on_update(on_update),
          m_is_connected(true) {
        m_connection = base.connect_match_signal(
          Foo(), [this] (auto is_match) {
            if(is_match) {
              m_on_update({m_base}, {});
            } else {
              m_is_connected = false;
              m_on_update({}, {m_base});
            }
          });
        if(base.is_match(Foo())) {
          m_on_update({m_base}, {});
        }
      }

      bool is_connected() const {
        return m_is_connected;
      }
    };
    return SelectConnection(
      std::make_unique<Executor>(base, on_update));
  }
}

namespace std {
  template<>
  struct hash<DisconnectingSelector> {
    std::size_t operator ()(const DisconnectingSelector&) const noexcept {
      return 0x7a3b9c1e;
    }
  };
}

TEST_SUITE("AndSelector") {
  TEST_CASE("equality") {
    REQUIRE(AndSelector(Foo(), Foo()) == AndSelector(Foo(), Foo()));
    REQUIRE(AndSelector(Foo(), Foo()) != AndSelector(Foo(), Bar()));
    REQUIRE(AndSelector(Foo(), Foo()) != AndSelector(Bar(), Foo()));
    REQUIRE(AndSelector(Foo(), Bar()) != AndSelector(Foo(), Foo()));
    REQUIRE(AndSelector(Bar(), Foo()) != AndSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AndSelector(Foo(), Bar()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(w1, Foo());
      REQUIRE(updates.empty());
      unmatch(w1, Foo());
      REQUIRE(updates.empty());
      match(w1, Bar());
      REQUIRE(updates.empty());
      match(w1, Foo());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
      }
      match(w1, Bar());
      REQUIRE(updates.empty());
      unmatch(w1, Foo());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(w1)));
      }
      unmatch(w1, Bar());
      REQUIRE(updates.empty());
    });
  }

  TEST_CASE("disconnect_during_update") {
    run_test([] {
      auto parent = QWidget();
      auto child = new QWidget(&parent);
      auto style = StyleSheet();
      style.get(ChildSelector(Any(),
        AndSelector(DisconnectingSelector(), Bar()))).
        set(ColorProperty(QColor(255, 0, 0)));
      set_style(parent, style);
      match(*child, Foo());
      match(*child, Bar());
      unmatch(*child, Foo());
    });
  }

  TEST_CASE("select_initialization") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Foo());
      match(w1, Bar());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AndSelector(Foo(), Bar()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.size() == 1);
      auto matches = updates.front();
      updates.pop_front();
      REQUIRE(matches.m_additions.size() == 1);
      REQUIRE(matches.m_removals.empty());
      REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
    });
  }
}
