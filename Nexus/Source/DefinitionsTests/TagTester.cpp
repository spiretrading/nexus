#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Tag.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  template<typename T>
  void require_tag(int type_id, int key, const T& value) {
    auto tag_value = Tag::Type(value);
    auto tag = Tag(key, tag_value);
    REQUIRE(tag.get_key() == key);
    REQUIRE(get<T>(tag.get_value()) == value);
    REQUIRE(tag.get_value().which() == type_id);
    REQUIRE(
      to_string(tag) == "(" + to_string(key) + " " + to_string(value) + ")");
    test_round_trip_shuttle(tag);
  }
}

TEST_SUITE("Tag") {
  TEST_CASE("types") {
    require_tag(Tag::INT_INDEX, 1, 42);
    require_tag(Tag::DOUBLE_INDEX, 2, 22.5);
    require_tag(Tag::CHAR_INDEX, 3, 'x');
    require_tag(Tag::STRING_INDEX, 4, std::string("hello"));
    require_tag(Tag::DATE_INDEX, 5, date(2025, 6, 30));
    require_tag(Tag::DURATION_INDEX, 6, time_duration(1, 2, 3));
    require_tag(
      Tag::DATE_TIME_INDEX, 7, time_from_string("2025-06-30 12:34:56"));
    require_tag(Tag::QUANTITY_INDEX, 8, Quantity(123));
    require_tag(Tag::MONEY_INDEX, 9, Money(567));
  }
}
