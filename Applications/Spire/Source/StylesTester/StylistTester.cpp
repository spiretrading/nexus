#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/FlipSelector.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("Stylist") {
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
}
