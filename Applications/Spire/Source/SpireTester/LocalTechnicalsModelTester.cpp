#include <doctest/doctest.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional_io.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  TimeAndSale test_sale(int price, int volume) {
    return TimeAndSale(boost::posix_time::ptime(), Money(price * Money::ONE),
      volume, TimeAndSale::Condition(), "null", "M1", "M2");
  }
}

TEST_SUITE("LocalTechnicalsModel") {
  TEST_CASE("signals") {
    auto model = LocalTechnicalsModel(Security());
    auto test_high = Money();
    auto test_low = Money();
    auto test_open = Money();
    auto test_close = Money();
    auto test_last = Money();
    auto test_volume = Quantity();
    model.connect_high_slot([&] (auto high) { test_high = high; });
    model.connect_low_slot([&] (auto low) { test_low = low; });
    model.connect_open_slot([&] (auto open) { test_open = open; });
    model.connect_close_slot([&] (auto close) { test_close = close; });
    model.connect_last_price_slot([&] (auto last) { test_last = last; });
    model.connect_volume_slot([&] (auto volume) { test_volume = volume; });
    model.update(test_sale(10, 100));
    REQUIRE(test_high == Money(10 * Money::ONE));
    REQUIRE(test_low == Money(10 * Money::ONE));
    REQUIRE(test_open == Money(10 * Money::ONE));
    REQUIRE(test_close == Money());
    REQUIRE(test_last == Money(10 * Money::ONE));
    REQUIRE(test_volume == Quantity(100));
    model.update(test_sale(15, 100));
    REQUIRE(test_high == Money(15 * Money::ONE));
    REQUIRE(test_low == Money(10 * Money::ONE));
    REQUIRE(test_open == Money(10 * Money::ONE));
    REQUIRE(test_close == Money());
    REQUIRE(test_last == Money(15 * Money::ONE));
    REQUIRE(test_volume == Quantity(200));
    model.update(test_sale(5, 100));
    REQUIRE(test_high == Money(15 * Money::ONE));
    REQUIRE(test_low == Money(5 * Money::ONE));
    REQUIRE(test_open == Money(10 * Money::ONE));
    REQUIRE(test_close == Money());
    REQUIRE(test_last == Money(5 * Money::ONE));
    REQUIRE(test_volume == Quantity(300));
    model.update(test_sale(7, 100));
    REQUIRE(test_high == Money(15 * Money::ONE));
    REQUIRE(test_low == Money(5 * Money::ONE));
    REQUIRE(test_open == Money(10 * Money::ONE));
    REQUIRE(test_close == Money());
    REQUIRE(test_last == Money(7 * Money::ONE));
    REQUIRE(test_volume == Quantity(400));
    model.set_close(Money(7 * Money::ONE));
    REQUIRE(test_high == Money(15 * Money::ONE));
    REQUIRE(test_low == Money(5 * Money::ONE));
    REQUIRE(test_open == Money(10 * Money::ONE));
    REQUIRE(test_close == Money(7 * Money::ONE));
    REQUIRE(test_last == Money(7 * Money::ONE));
    REQUIRE(test_volume == Quantity(400));
  }

  TEST_CASE("signals_emit_only_as_required") {
    auto model = LocalTechnicalsModel(Security());
    auto test_high = std::optional<Money>();
    auto test_low = std::optional<Money>();
    auto test_open = std::optional<Money>();
    auto test_close = std::optional<Money>();
    auto test_last = std::optional<Money>();
    auto reset_values = [&] {
      test_high.reset();
      test_low.reset();
      test_open.reset();
      test_close.reset();
      test_last.reset();
    };
    model.connect_high_slot([&] (auto high) { test_high = high; });
    model.connect_low_slot([&] (auto low) { test_low = low; });
    model.connect_open_slot([&] (auto open) { test_open = open; });
    model.connect_close_slot([&] (auto close) { test_close = close; });
    model.connect_last_price_slot([&] (auto last) { test_last = last; });
    model.update(test_sale(10, 100));
    REQUIRE(test_high == Money(10 * Money::ONE));
    REQUIRE(test_low == Money(10 * Money::ONE));
    REQUIRE(test_open == Money(10 * Money::ONE));
    REQUIRE(!test_close);
    REQUIRE(test_last == Money(10 * Money::ONE));
    reset_values();
    model.update(test_sale(15, 100));
    REQUIRE(test_high == Money(15 * Money::ONE));
    REQUIRE(!test_low);
    REQUIRE(!test_open);
    REQUIRE(!test_close);
    REQUIRE(test_last == Money(15 * Money::ONE));
    reset_values();
    model.update(test_sale(5, 100));
    REQUIRE(!test_high);
    REQUIRE(test_low == Money(5 * Money::ONE));
    REQUIRE(!test_open);
    REQUIRE(!test_close);
    REQUIRE(test_last == Money(5 * Money::ONE));
    reset_values();
    model.update(test_sale(5, 100));
    REQUIRE(!test_high);
    REQUIRE(!test_low);
    REQUIRE(!test_open);
    REQUIRE(!test_close);
    REQUIRE(!test_last);
    reset_values();
    model.set_close(Money(5 * Money::ONE));
    REQUIRE(!test_high);
    REQUIRE(!test_low);
    REQUIRE(!test_open);
    REQUIRE(test_close == Money(5 * Money::ONE));
    REQUIRE(!test_last);
  }

  TEST_CASE("setting_and_getting_values") {
    auto model = LocalTechnicalsModel(Security());
    model.update(test_sale(10, 100));
    REQUIRE(model.get_high() == Money(10 * Money::ONE));
    REQUIRE(model.get_low() == Money(10 * Money::ONE));
    REQUIRE(model.get_open() == Money(10 * Money::ONE));
    REQUIRE(!model.get_close());
    REQUIRE(model.get_last_price() == Money(10 * Money::ONE));
    REQUIRE(model.get_volume() == Quantity(100));
    model.update(test_sale(5, 100));
    REQUIRE(model.get_high() == Money(10 * Money::ONE));
    REQUIRE(model.get_low() == Money(5 * Money::ONE));
    REQUIRE(model.get_open() == Money(10 * Money::ONE));
    REQUIRE(!model.get_close());
    REQUIRE(model.get_last_price() == Money(5 * Money::ONE));
    REQUIRE(model.get_volume() == Quantity(200));
    model.set_close(Money(5 * Money::ONE));
    REQUIRE(model.get_high() == Money(10 * Money::ONE));
    REQUIRE(model.get_low() == Money(5 * Money::ONE));
    REQUIRE(model.get_open() == Money(10 * Money::ONE));
    REQUIRE(model.get_close() == Money(5 * Money::ONE));
    REQUIRE(model.get_last_price() == Money(5 * Money::ONE));
    REQUIRE(model.get_volume() == Quantity(200));
  }
}
