#include <deque>
#include <memory>
#include <boost/signals2/connection.hpp>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("Stylist") {
  UI_TEST_CASE("proxy_apply_unapply") {
    auto parent = QWidget();
    auto child = QWidget(&parent);
    match(parent, Foo());
    proxy_style(parent, child);
    auto style = StyleSheet();
    style.get(Foo()).set(BackgroundColor(QColor(0xFF00FF)));
    set_style(parent, style);
    auto computed_block = std::deque<Block>();
    connect_style_signal(child, [&] {
      auto& stylist = find_stylist(child);
      computed_block.push_back(stylist.get_computed_block());
    });
    set_style(parent, {});
  }

  UI_TEST_CASE("proxy_cycle") {
    auto parent = QWidget();
    auto child = QWidget();
    proxy_style(parent, child);
    auto style = StyleSheet();
    style.get(+Any() << Any()).set(Visibility::INVISIBLE);
    set_style(child, std::move(style));
    child.setParent(&parent);
  }

  UI_TEST_CASE("parent_priority") {
    auto parent = QWidget();
    auto parent_style = StyleSheet();
    parent_style.get(ChildSelector(Any(), Any())).
      set(BackgroundColor(QColor(0x00FF00)));
    set_style(parent, std::move(parent_style));
    auto child = QWidget(&parent);
    auto child_style = StyleSheet();
    child_style.get(Any()).set(BackgroundColor(QColor(0xFF0000)));
    set_style(child, std::move(child_style));
    auto block = find_stylist(child).get_computed_block();
    auto color = find<BackgroundColor>(block);
    REQUIRE(color.is_initialized());
    REQUIRE(*color == QColor(0x00FF00));
  }

  UI_TEST_CASE("deferred_parent_priority") {
    auto parent = QWidget();
    auto parent_style = StyleSheet();
    parent_style.get(ChildSelector(Any(), Any())).
      set(BackgroundColor(QColor(0x00FF00)));
    set_style(parent, std::move(parent_style));
    auto child = QWidget();
    auto child_style = StyleSheet();
    child_style.get(Any()).set(BackgroundColor(QColor(0xFF0000)));
    set_style(child, std::move(child_style));
    child.setParent(&parent);
    auto block = find_stylist(child).get_computed_block();
    auto color = find<BackgroundColor>(block);
    REQUIRE(color.is_initialized());
    REQUIRE(*color == QColor(0x00FF00));
  }

  UI_TEST_CASE("reselection_during_rule_removal") {
    auto widget = QWidget();
    match(widget, Foo());
    auto style = StyleSheet();
    style.get(Foo()).set(IntProperty(42));
    set_style(widget, std::move(style));
    auto is_triggered = false;
    connect_style_signal(widget, [&] {
      if(!is_triggered) {
        is_triggered = true;
        unmatch(widget, Foo());
        match(widget, Foo());
      }
    });
    set_style(widget, {});
    auto update = StyleSheet();
    update.get(Any()).set(ColorProperty(QColor(0xFF0000)));
    set_style(widget, std::move(update));
    auto block = find_stylist(widget).get_computed_block();
    REQUIRE(find<ColorProperty>(block).is_initialized());
    REQUIRE(!find<IntProperty>(block).is_initialized());
  }

  UI_TEST_CASE("style_signal") {
    auto widget = QWidget();
    auto style = StyleSheet();
    style.get(Any()).set(IntProperty(1));
    set_style(widget, style);
    auto is_styled = false;
    auto connection = scoped_connection(connect_style_signal(widget, [&] {
      is_styled = true;
    }));
    style.get(Any()).set(IntProperty(2));
    set_style(widget, std::move(style));
    REQUIRE(is_styled);
  }

  UI_TEST_CASE("match_signal") {
    auto widget = QWidget();
    auto& stylist = find_stylist(widget);
    auto foo_updates = std::deque<bool>();
    auto foo_connection = scoped_connection(
      stylist.connect_match_signal(Foo(), [&] (auto is_match) {
        foo_updates.push_back(is_match);
      }));
    auto bar_updates = std::deque<bool>();
    auto bar_connection = scoped_connection(
      stylist.connect_match_signal(Bar(), [&] (auto is_match) {
        bar_updates.push_back(is_match);
      }));
    match(widget, Foo());
    REQUIRE(foo_updates.size() == 1);
    REQUIRE(foo_updates.front());
    foo_updates.pop_front();
    REQUIRE(bar_updates.empty());
    match(widget, Foo());
    REQUIRE(foo_updates.empty());
    match(widget, Bar());
    REQUIRE(foo_updates.empty());
    REQUIRE(bar_updates.size() == 1);
    REQUIRE(bar_updates.front());
    bar_updates.pop_front();
    unmatch(widget, Foo());
    REQUIRE(foo_updates.size() == 1);
    REQUIRE(!foo_updates.front());
    foo_updates.pop_front();
    REQUIRE(bar_updates.empty());
    unmatch(widget, Foo());
    REQUIRE(foo_updates.empty());
    foo_connection.disconnect();
    match(widget, Foo());
    REQUIRE(foo_updates.empty());
    unmatch_all(widget);
    REQUIRE(foo_updates.empty());
    REQUIRE(bar_updates.size() == 1);
    REQUIRE(!bar_updates.front());
    bar_updates.pop_front();
  }

  UI_TEST_CASE("match_signal_reentrancy") {
    auto widget = QWidget();
    auto& stylist = find_stylist(widget);
    auto bar_updates = std::deque<bool>();
    auto bar_connection = scoped_connection(
      stylist.connect_match_signal(Bar(), [&] (auto is_match) {
        bar_updates.push_back(is_match);
      }));
    auto qux_updates = std::deque<bool>();
    auto qux_connection = scoped_connection();
    auto foo_connection = scoped_connection(
      stylist.connect_match_signal(Foo(), [&] (auto is_match) {
        match(widget, Bar());
        qux_connection = stylist.connect_match_signal(Qux(),
          [&] (auto is_match) {
            qux_updates.push_back(is_match);
          });
      }));
    match(widget, Foo());
    REQUIRE(bar_updates.size() == 1);
    REQUIRE(bar_updates.front());
    bar_updates.pop_front();
    REQUIRE(qux_updates.empty());
    match(widget, Qux());
    REQUIRE(qux_updates.size() == 1);
    REQUIRE(qux_updates.front());
    qux_updates.pop_front();
  }

  UI_TEST_CASE("match_signal_tracking") {
    auto widget = QWidget();
    auto& stylist = find_stylist(widget);
    auto updates = std::deque<bool>();
    auto tracker = std::make_shared<int>(0);
    auto connection = scoped_connection(stylist.connect_match_signal(Foo(),
      Stylist::MatchSignal::slot_type([&] (auto is_match) {
        updates.push_back(is_match);
      }).track_foreign(tracker)));
    match(widget, Foo());
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front());
    updates.pop_front();
    tracker.reset();
    unmatch(widget, Foo());
    REQUIRE(updates.empty());
    REQUIRE(!connection.connected());
  }

  UI_TEST_CASE("link_signals") {
    auto root = QWidget();
    auto target = QWidget();
    auto other = QWidget();
    auto links = std::deque<const Stylist*>();
    auto link_connection = scoped_connection(
      find_stylist(root).connect_link_signal([&] (const auto& stylist) {
        links.push_back(&stylist);
      }));
    auto backlinks = std::deque<const Stylist*>();
    auto backlink_connection = scoped_connection(
      find_stylist(target).connect_backlink_signal(
        [&] (const auto& stylist) {
          backlinks.push_back(&stylist);
        }));
    link(root, target);
    REQUIRE(links.size() == 1);
    REQUIRE(links.front() == &find_stylist(target));
    links.pop_front();
    REQUIRE(backlinks.size() == 1);
    REQUIRE(backlinks.front() == &find_stylist(root));
    backlinks.pop_front();
    link(other, target);
    REQUIRE(links.empty());
    REQUIRE(backlinks.size() == 1);
    REQUIRE(backlinks.front() == &find_stylist(other));
    backlinks.pop_front();
    link(root, other);
    REQUIRE(links.size() == 1);
    REQUIRE(links.front() == &find_stylist(other));
    links.pop_front();
    REQUIRE(backlinks.empty());
  }

  UI_TEST_CASE("delete_signal") {
    auto widget = new QWidget();
    auto is_deleted = false;
    find_stylist(*widget).connect_delete_signal([&] {
      is_deleted = true;
    });
    REQUIRE(!is_deleted);
    delete widget;
    REQUIRE(is_deleted);
  }
}
