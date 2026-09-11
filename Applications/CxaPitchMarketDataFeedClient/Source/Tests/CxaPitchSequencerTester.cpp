#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSequencer.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  int read(CxaPitchSequencer& sequencer) {
    auto payload = sequencer.read();
    if(!payload) {
      return 0;
    }
    return CxaPitchMessage::parse(
      std::string_view(payload->get_data(), payload->get_size())).m_type;
  }

  std::string encode_block(
      std::uint32_t sequence, const std::vector<std::uint8_t>& types) {
    auto payload = std::string();
    for(auto type : types) {
      payload += char(6);
      payload += static_cast<char>(type);
      payload.append(4, char(0));
    }
    auto header = CxaPitchHeader();
    header.m_length = static_cast<std::uint16_t>(
      CxaPitchHeader::LENGTH + payload.size());
    header.m_count = static_cast<std::uint8_t>(types.size());
    header.m_unit = 1;
    header.m_sequence = sequence;
    auto block = Beam::SharedBuffer();
    header.encode(Beam::out(block));
    return std::string(block.get_data(), block.get_size()) + payload;
  }

  std::string encode_timed_block(
      std::uint32_t sequence, std::uint64_t timestamp) {
    auto payload = std::string();
    payload += static_cast<char>(CxaPitchDeleteOrder::LENGTH);
    payload += static_cast<char>(CxaPitchDeleteOrder::TYPE);
    for(auto i = 0; i != 8; ++i) {
      payload += static_cast<char>((timestamp >> (8 * i)) & 0xFF);
    }
    payload.append(8, char(0));
    auto header = CxaPitchHeader();
    header.m_length = static_cast<std::uint16_t>(
      CxaPitchHeader::LENGTH + payload.size());
    header.m_count = 1;
    header.m_unit = 1;
    header.m_sequence = sequence;
    auto block = Beam::SharedBuffer();
    header.encode(Beam::out(block));
    return std::string(block.get_data(), block.get_size()) + payload;
  }
}

TEST_SUITE("CxaPitchSequencer") {
  TEST_CASE("unit_clear_sequence") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(100, {0x11});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    SUBCASE("expected") {
      auto clear = encode_block(101, {CxaPitchUnitClear::TYPE});
      sequencer.add(0, CxaPitchBlock::parse(clear), timestamp);
      REQUIRE(read(sequencer) == CxaPitchUnitClear::TYPE);
      REQUIRE(sequencer.get_sequence().value_or(0) == 102);
    }
    SUBCASE("ahead") {
      auto clear = encode_block(102, {CxaPitchUnitClear::TYPE});
      sequencer.add(0, CxaPitchBlock::parse(clear), timestamp);
      sequencer.add(1, CxaPitchBlock::parse(clear), timestamp);
      REQUIRE(read(sequencer) == 0);
      auto gap = sequencer.get_gap();
      REQUIRE(gap.has_value());
      REQUIRE(gap->m_sequence == 101);
      REQUIRE(gap->m_count == 1);
      auto missing = encode_block(101, {0x12});
      sequencer.recover(CxaPitchBlock::parse(missing));
      REQUIRE(read(sequencer) == 0x12);
      REQUIRE(read(sequencer) == CxaPitchUnitClear::TYPE);
      REQUIRE(sequencer.get_sequence().value_or(0) == 103);
    }
    REQUIRE(read(sequencer) == 0);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("lower_sequences") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_timed_block(100, 1500000000000000000);
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == CxaPitchDeleteOrder::TYPE);
    auto earlier = std::string();
    SUBCASE("unit_clear") {
      earlier = encode_block(1, {CxaPitchUnitClear::TYPE});
    }
    SUBCASE("newer_timestamp") {
      earlier = encode_timed_block(1, 1500000001000000000);
    }
    sequencer.add(0, CxaPitchBlock::parse(earlier), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(earlier), timestamp);
    auto second = encode_timed_block(2, 1500000002000000000);
    sequencer.add(0, CxaPitchBlock::parse(second), timestamp);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 101);
    REQUIRE(sequencer.get_position() == 101);
    REQUIRE(!sequencer.get_gap());
    auto next = encode_timed_block(101, 1500000003000000000);
    sequencer.add(0, CxaPitchBlock::parse(next), timestamp);
    REQUIRE(read(sequencer) == CxaPitchDeleteOrder::TYPE);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 102);
  }

  TEST_CASE("ignore_duplicate_block") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(100, {0x11});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    auto next = encode_block(101, {0x12});
    sequencer.add(1, CxaPitchBlock::parse(next), timestamp);
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x12);
  }

  TEST_CASE("ignore_duplicate_block_on_each_feed") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_timed_block(100, 1500000000000000000);
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == CxaPitchDeleteOrder::TYPE);
    REQUIRE(read(sequencer) == 0);
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 101);
  }

  TEST_CASE("duplicate_unit_clear_on_each_feed") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto clear = encode_block(100, {CxaPitchUnitClear::TYPE});
    for(auto feed = 0; feed != 2; ++feed) {
      sequencer.add(feed, CxaPitchBlock::parse(clear), timestamp);
    }
    REQUIRE(read(sequencer) == CxaPitchUnitClear::TYPE);
    auto buffered = encode_timed_block(102, 1500000000000000000);
    for(auto feed = 0; feed != 2; ++feed) {
      sequencer.add(feed, CxaPitchBlock::parse(buffered), timestamp);
    }
    for(auto feed = 0; feed != 2; ++feed) {
      sequencer.add(feed, CxaPitchBlock::parse(clear), timestamp);
      REQUIRE(read(sequencer) == 0);
      REQUIRE(sequencer.get_sequence().value_or(0) == 101);
      auto gap = sequencer.get_gap();
      REQUIRE(gap.has_value());
      REQUIRE(gap->m_sequence == 101);
      REQUIRE(gap->m_count == 1);
    }
    auto recovery = encode_timed_block(101, 1499999999000000000);
    sequencer.recover(CxaPitchBlock::parse(recovery));
    REQUIRE(read(sequencer) == CxaPitchDeleteOrder::TYPE);
    REQUIRE(read(sequencer) == CxaPitchDeleteOrder::TYPE);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 103);
  }

  TEST_CASE("untimestamped_replay") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto type = CxaPitchEndOfSession::TYPE;
    auto replay = std::string();
    SUBCASE("delayed_heartbeat") {
      replay = encode_block(100, {});
    }
    SUBCASE("duplicate_message") {
      replay = encode_block(100, {type});
    }
    SUBCASE("duplicate_unit_clear") {
      type = CxaPitchUnitClear::TYPE;
      replay = encode_block(100, {type});
    }
    auto first = encode_block(100, {type});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == type);
    for(auto i = 0; i != 2; ++i) {
      sequencer.add(0, CxaPitchBlock::parse(replay), timestamp);
      REQUIRE(read(sequencer) == 0);
      REQUIRE(sequencer.get_sequence().value_or(0) == 101);
      REQUIRE(sequencer.get_position() == 101);
      REQUIRE(!sequencer.get_gap());
    }
  }

  TEST_CASE("leading_feed_position") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    sequencer.add(0, CxaPitchBlock::parse(encode_block(1, {0x11})), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(encode_block(1, {0x11})), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    sequencer.add(
      0, CxaPitchBlock::parse(encode_block(1000000, {0x12})), timestamp);
    REQUIRE(sequencer.get_position() == 2);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("read_messages_in_order") {
    auto sequencer = CxaPitchSequencer(1, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto data = encode_block(1, {0x11, 0x12, 0x13});
    sequencer.add(0, CxaPitchBlock::parse(data), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
    REQUIRE(read(sequencer) == 0x13);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("arbitrate_duplicate_messages") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
    REQUIRE(read(sequencer) == 0);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(!sequencer.get_gap());
    auto third = encode_block(3, {0x13});
    sequencer.add(1, CxaPitchBlock::parse(third), timestamp);
    REQUIRE(read(sequencer) == 0x13);
    REQUIRE(read(sequencer) == 0x14);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("overlapping_blocks") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    auto buffered = encode_block(5, {0x15, 0x16});
    sequencer.add(0, CxaPitchBlock::parse(buffered), timestamp);
    auto overlap = encode_block(1, {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17});
    SUBCASE("redundant_feed") {
      sequencer.add(1, CxaPitchBlock::parse(overlap), timestamp);
    }
    SUBCASE("recovery") {
      sequencer.recover(CxaPitchBlock::parse(overlap));
    }
    for(auto type = 0x12; type != 0x18; ++type) {
      REQUIRE(read(sequencer) == type);
    }
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 8);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("gap_quorum") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    REQUIRE(!sequencer.get_gap());
    sequencer.add(1, CxaPitchBlock::parse(fourth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
  }

  TEST_CASE("three_feed_quorum") {
    auto sequencer = CxaPitchSequencer(3, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11});
    for(auto feed = 0; feed != 3; ++feed) {
      sequencer.add(feed, CxaPitchBlock::parse(first), timestamp);
    }
    REQUIRE(read(sequencer) == 0x11);
    auto slowest = 0;
    SUBCASE("first_feed") {
      slowest = 0;
    }
    SUBCASE("second_feed") {
      slowest = 1;
    }
    SUBCASE("third_feed") {
      slowest = 2;
    }
    sequencer.add(
      slowest, CxaPitchBlock::parse(encode_block(4, {})), timestamp);
    auto later = timestamp + duration_from_string("00:00:02");
    sequencer.add((slowest + 1) % 3,
      CxaPitchBlock::parse(encode_block(6, {})), later);
    sequencer.add((slowest + 2) % 3,
      CxaPitchBlock::parse(encode_block(8, {})), later);
    REQUIRE(sequencer.get_position() == 4);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 2);
    later = timestamp + duration_from_string("00:00:04");
    sequencer.update(later);
    REQUIRE(sequencer.get_position() == 6);
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 4);
    sequencer.add(slowest, CxaPitchBlock::parse(encode_block(5, {})), later);
    REQUIRE(sequencer.get_position() == 5);
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 3);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 2);
  }

  TEST_CASE("heartbeat_gap") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
    auto fifth = encode_block(5, {0x15});
    sequencer.add(0, CxaPitchBlock::parse(fifth), timestamp);
    REQUIRE(!sequencer.get_gap());
    auto heartbeat = encode_block(5, {});
    sequencer.add(1, CxaPitchBlock::parse(heartbeat), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 2);
  }

  TEST_CASE("ignore_unsequenced_heartbeat") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    auto heartbeat = encode_block(0, {});
    sequencer.add(1, CxaPitchBlock::parse(heartbeat), timestamp);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("feed_timeout") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
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
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
    auto third = encode_block(3, {0x13});
    sequencer.add(1, CxaPitchBlock::parse(third), later);
    REQUIRE(read(sequencer) == 0x13);
    REQUIRE(read(sequencer) == 0x14);
    REQUIRE(read(sequencer) == 0x15);
    auto seventh = encode_block(7, {0x17});
    sequencer.add(0, CxaPitchBlock::parse(seventh), later);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("stalled_feed_replay") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = std::string();
    auto second = std::string();
    auto fifth = std::string();
    auto type = 0;
    SUBCASE("untimestamped") {
      first = encode_block(1, {0x11});
      second = encode_block(2, {0x11});
      fifth = encode_block(5, {0x15});
      type = 0x11;
    }
    SUBCASE("timestamped") {
      first = encode_timed_block(1, 1500000000000000000);
      second = encode_timed_block(2, 1500000000000000000);
      fifth = encode_timed_block(5, 1500000001000000000);
      type = CxaPitchDeleteOrder::TYPE;
    }
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == type);
    auto later = timestamp + duration_from_string("00:00:04");
    sequencer.add(0, CxaPitchBlock::parse(fifth), later);
    REQUIRE(sequencer.get_gap().has_value());
    auto heartbeat = encode_block(6, {});
    for(auto i = 0; i != 5; ++i) {
      sequencer.add(0, CxaPitchBlock::parse(heartbeat), later);
      sequencer.add(1, CxaPitchBlock::parse(first), later);
      auto gap = sequencer.get_gap();
      REQUIRE(gap.has_value());
      REQUIRE(gap->m_sequence == 2);
      REQUIRE(gap->m_count == 3);
      REQUIRE(read(sequencer) == 0);
      later += duration_from_string("00:00:01");
    }
    sequencer.add(1, CxaPitchBlock::parse(second), later);
    REQUIRE(read(sequencer) == type);
    REQUIRE(sequencer.get_sequence().value_or(0) == 3);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("single_feed_gap") {
    auto sequencer = CxaPitchSequencer(1, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11, 0x12});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    REQUIRE(read(sequencer) == 0x12);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 3);
    REQUIRE(gap->m_count == 1);
  }

  TEST_CASE("reset_sequence") {
    auto sequencer = CxaPitchSequencer(1, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto data = encode_block(1, {0x11, 0x12, 0x13});
    sequencer.add(0, CxaPitchBlock::parse(data), timestamp);
    sequencer.reset(2);
    REQUIRE(read(sequencer) == 0x12);
    REQUIRE(read(sequencer) == 0x13);
    REQUIRE(read(sequencer) == 0);
  }

  TEST_CASE("recovery_after_skip") {
    auto sequencer = CxaPitchSequencer(1, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    auto sixth = encode_block(6, {0x16});
    sequencer.add(0, CxaPitchBlock::parse(sixth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 2);
    sequencer.reset(4);
    REQUIRE(read(sequencer) == 0x14);
    REQUIRE(read(sequencer) == 0);
    auto skipped = encode_block(2, {0x12, 0x13});
    sequencer.recover(CxaPitchBlock::parse(skipped));
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 5);
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 5);
    REQUIRE(gap->m_count == 1);
    auto overlap = encode_block(2, {0x12, 0x13, 0x14, 0x15, 0x16, 0x17});
    sequencer.recover(CxaPitchBlock::parse(overlap));
    REQUIRE(read(sequencer) == 0x15);
    REQUIRE(read(sequencer) == 0x16);
    REQUIRE(read(sequencer) == 0x17);
    REQUIRE(read(sequencer) == 0);
    REQUIRE(sequencer.get_sequence().value_or(0) == 8);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("recover_confirmed_gap") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    auto fourth = encode_block(4, {0x14});
    sequencer.add(0, CxaPitchBlock::parse(fourth), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(fourth), timestamp);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 2);
    auto replay = encode_block(2, {0x12, 0x13});
    sequencer.recover(CxaPitchBlock::parse(replay));
    REQUIRE(read(sequencer) == 0x12);
    REQUIRE(read(sequencer) == 0x13);
    REQUIRE(read(sequencer) == 0x14);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("recovery_quorum") {
    auto sequencer = CxaPitchSequencer(2, duration_from_string("00:00:03"));
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto first = encode_block(1, {0x11});
    sequencer.add(0, CxaPitchBlock::parse(first), timestamp);
    sequencer.add(1, CxaPitchBlock::parse(first), timestamp);
    REQUIRE(read(sequencer) == 0x11);
    auto replay = encode_block(3, {0x13});
    sequencer.recover(CxaPitchBlock::parse(replay));
    REQUIRE(read(sequencer) == 0);
    REQUIRE(!sequencer.get_gap());
  }

  TEST_CASE("recovery_before_initialization") {
    auto sequencer = CxaPitchSequencer(1, duration_from_string("00:00:03"));
    auto replay = encode_block(1, {0x11});
    sequencer.recover(CxaPitchBlock::parse(replay));
    REQUIRE(read(sequencer) == 0);
    auto timestamp = time_from_string("2026-09-08 10:00:00");
    auto second = encode_block(2, {0x12});
    sequencer.add(0, CxaPitchBlock::parse(second), timestamp);
    REQUIRE(read(sequencer) == 0x12);
  }

}
