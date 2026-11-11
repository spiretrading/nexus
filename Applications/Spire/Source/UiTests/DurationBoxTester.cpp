#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include <QApplication>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Tests;

TEST_SUITE("DurationBox") {
  UI_TEST_CASE("reenabled_text_color") {
    auto box = DurationBox(std::make_shared<LocalOptionalDurationModel>(
      duration_from_string("19:19:19.199")));
    box.show();
    QApplication::processEvents();
    auto text_boxes = box.findChildren<TextBox*>();
    REQUIRE(!text_boxes.empty());
    auto colors = std::vector<optional<TextColor>>();
    for(auto text_box : text_boxes) {
      colors.push_back(find<TextColor>(get_computed_block(*text_box)));
    }
    box.setEnabled(false);
    QApplication::processEvents();
    box.setEnabled(true);
    QApplication::processEvents();
    for(auto i = std::size_t(0); i != text_boxes.size(); ++i) {
      REQUIRE((find<TextColor>(get_computed_block(*text_boxes[i])) ==
        colors[i]));
    }
  }
}
