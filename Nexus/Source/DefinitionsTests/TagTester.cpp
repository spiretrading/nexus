#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Tag.hpp"

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
    auto ss = std::stringstream();
    ss << tag;
    REQUIRE(ss.str() == "(" + lexical_cast<std::string>(key) + " " +
      lexical_cast<std::string>(value) + ")");
    Beam::Serialization::Tests::TestRoundTripShuttle(tag);
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
    require_tag(Tag::DATE_TIME_INDEX, 7,
      ptime(date(2025, 6, 30), time_duration(12, 34, 56)));
    require_tag(Tag::QUANTITY_INDEX, 8, Quantity(123));
    require_tag(Tag::MONEY_INDEX, 9, Money(567));
  }
}
