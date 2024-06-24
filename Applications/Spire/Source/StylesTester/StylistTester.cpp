#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("Stylist") {
  TEST_CASE("proxy_apply_unapply") {
    run_test([] {
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
    });
  }

  TEST_CASE("proxy_cycle") {
    run_test([] {
      auto parent = QWidget();
      auto child = QWidget();
      proxy_style(parent, child);
      auto style = StyleSheet();
      style.get(+Any() << Any()).set(Visibility::INVISIBLE);
      set_style(child, std::move(style));
      child.setParent(&parent);
    });
  }

  TEST_CASE("parent_priority") {
    run_test([] {
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
    });
  }

  TEST_CASE("deferred_parent_priority") {
    run_test([] {
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
    });
  }
}
