#include <doctest/doctest.h>
#include "Spire/Styles/BoxStyles.hpp"
#include "Spire/Styles/StyleSheet.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("StyleSheet") {
  TEST_CASE("dummy") {
    auto styles = StyleSheet();
    styles.get(Any()).get_block().set(border(1, QColor::fromRgb(0, 0, 255)));
  }
}
