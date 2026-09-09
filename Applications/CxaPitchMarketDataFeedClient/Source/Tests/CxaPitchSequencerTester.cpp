#include <cstdint>
#include <string>
#include <vector>
#include <doctest/doctest.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSequencer.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::string encode_block(
      std::uint32_t sequence, const std::vector<std::uint8_t>& types) {
    auto payload = std::string();
    for(auto type : types) {
      payload += char(6);
      payload += static_cast<char>(type);
      payload.append(4, char(0));
    }
    auto length = static_cast<std::uint16_t>(
      CxaPitchHeader::LENGTH + payload.size());
    auto block = std::string();
    block += static_cast<char>(length & 0xFF);
    block += static_cast<char>((length >> 8) & 0xFF);
    block += static_cast<char>(types.size());
    block += char(1);
    block += static_cast<char>(sequence & 0xFF);
    block += static_cast<char>((sequence >> 8) & 0xFF);
    block += static_cast<char>((sequence >> 16) & 0xFF);
    block += static_cast<char>((sequence >> 24) & 0xFF);
    return block + payload;
  }
}

TEST_SUITE("CxaPitchSequencer") {
  TEST_CASE("read_messages_in_order") {
    auto sequencer =
      CxaPitchSequencer(1, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto data = encode_block(1, {0x11, 0x12, 0x13});
    sequencer.add(0, CxaPitchBlock::parse(data), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    REQUIRE(sequencer.read()->m_type == 0x13);
    REQUIRE(!sequencer.read());
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("arbitrate_duplicates_and_recover_a_loss") {
    auto sequencer =
      CxaPitchSequencer(2, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    REQUIRE(!sequencer.read());
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    REQUIRE(!sequencer.read());
    REQUIRE(!sequencer.get_gap());
    auto third = encode_block(3, {0x13});
    sequencer.add(1, CxaPitchBlock::parse(third), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x13);
    REQUIRE(sequencer.read()->m_type == 0x14);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("confirm_a_gap_once_every_feed_passes_it") {
    auto sequencer =
      CxaPitchSequencer(2, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    REQUIRE(!sequencer.get_gap());
    sequencer.add(1, CxaPitchBlock::parse(fourth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
  }

  TEST_CASE("confirm_a_gap_from_a_heartbeat") {
    auto sequencer =
      CxaPitchSequencer(2, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    auto fifth = encode_block(5, {0x15});
    sequencer.add(0, CxaPitchBlock::parse(fifth), timestamp);
    REQUIRE(!sequencer.get_gap());
    auto heartbeat = encode_block(5, {});
    sequencer.add(1, CxaPitchBlock::parse(heartbeat), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 2);
  }

  TEST_CASE("ignore_an_unsequenced_heartbeat") {
    auto sequencer =
      CxaPitchSequencer(2, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    auto heartbeat = encode_block(0, {});
    sequencer.add(1, CxaPitchBlock::parse(heartbeat), timestamp);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("exclude_a_silent_feed_and_readmit_it") {
    auto sequencer =
      CxaPitchSequencer(2, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    REQUIRE(!sequencer.get_gap());
    auto fifth = encode_block(5, {0x15});
    sequencer.add(0, CxaPitchBlock::parse(fifth),
      timestamp + duration_from_string("00:00:02"));
    REQUIRE(!sequencer.get_gap());
    auto later = timestamp + duration_from_string("00:00:04");
    sequencer.update(later);
    auto gap = sequencer.get_gap();
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
    auto third = encode_block(3, {0x13});
    sequencer.add(1, CxaPitchBlock::parse(third), later);
    REQUIRE(sequencer.read()->m_type == 0x13);
    REQUIRE(sequencer.read()->m_type == 0x14);
    REQUIRE(sequencer.read()->m_type == 0x15);
    auto seventh = encode_block(7, {0x17});
    sequencer.add(0, CxaPitchBlock::parse(seventh), later);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("confirm_a_gap_immediately_with_one_feed") {
    auto sequencer =
      CxaPitchSequencer(1, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
  }

  TEST_CASE("confirm_a_gap_once_the_held_messages_reach_capacity") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"), 2);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(sequencer.read()->m_type == 0x11);
    REQUIRE(sequencer.read()->m_type == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    REQUIRE(!sequencer.get_gap());
    auto fifth = encode_block(5, {0x15});
    sequencer.add(0, CxaPitchBlock::parse(fifth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
  }

  TEST_CASE("reset_discards_earlier_messages") {
    auto sequencer =
      CxaPitchSequencer(1, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto data = encode_block(1, {0x11, 0x12, 0x13});
    sequencer.add(0, CxaPitchBlock::parse(data), timestamp);
    sequencer.reset(2);
    REQUIRE(sequencer.read()->m_type == 0x12);
    REQUIRE(sequencer.read()->m_type == 0x13);
    REQUIRE(!sequencer.read());
  }

  TEST_CASE("add_rejects_an_unknown_feed") {
    auto sequencer =
      CxaPitchSequencer(1, duration_from_string("00:00:03"), 16);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto data = encode_block(1, {0x11});
    REQUIRE_THROWS_AS(sequencer.add(1, CxaPitchBlock::parse(data), timestamp),
      CxaPitchParserException);
  }
}
