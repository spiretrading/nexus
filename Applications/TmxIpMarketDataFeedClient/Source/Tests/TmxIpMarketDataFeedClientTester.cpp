#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  struct MessageClient {
    Queue<std::string> m_messages;
    std::string m_payload;
    std::atomic_int m_count = 0;
    bool m_is_closed = false;

    StampMessage read() {
      m_payload = m_messages.pop();
      ++m_count;
      return StampMessage::parse(m_payload);
    }

    int get_count() const {
      return m_count;
    }

    void close() {
      m_is_closed = true;
      m_messages.close();
    }
  };
}

TEST_SUITE("TmxIpMarketDataFeedClient") {
  TEST_CASE("close") {
    auto source = MessageClient();
    {
      auto client = TmxIpMarketDataFeedClient(TmxIpConfiguration(), &source);
      flush_pending_routines();
      REQUIRE_FALSE(client.is_finished());
      SUBCASE("explicit") {
        client.close();
        REQUIRE(client.is_finished());
        REQUIRE_FALSE(client.get_exception());
        REQUIRE_NOTHROW(client.close());
      }
      SUBCASE("destructor") {}
    }
    REQUIRE(source.m_is_closed);
    REQUIRE(source.get_count() == 0);
  }

  TEST_CASE("source_error") {
    auto source = MessageClient();
    auto client = TmxIpMarketDataFeedClient(TmxIpConfiguration(), &source);
    flush_pending_routines();
    auto exception = std::make_exception_ptr(IOException("Feed failed."));
    source.m_messages.close(exception);
    flush_pending_routines();
    REQUIRE(client.is_finished());
    exception = client.get_exception();
    REQUIRE(exception);
    REQUIRE_THROWS_AS(std::rethrow_exception(exception), IOException);
    client.close();
    REQUIRE(source.m_is_closed);
    REQUIRE(client.get_exception() == exception);
  }

  TEST_CASE("parse_error") {
    auto source = MessageClient();
    auto client = TmxIpMarketDataFeedClient(TmxIpConfiguration(), &source);
    SUBCASE("stamp_framing") {
      source.m_messages.push("not a STAMP message");
    }
    SUBCASE("required_field") {
      source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
        "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "191=25.50"
        "\x1e" "57=20260920090000000");
    }
    SUBCASE("price") {
      source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
        "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
        "\x1e" "191=invalid\x1e" "57=20260920090000000");
    }
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=FutureMessage");
    flush_pending_routines();
    REQUIRE(client.is_finished());
    REQUIRE(source.get_count() == 1);
    auto exception = client.get_exception();
    REQUIRE(exception);
    REQUIRE_THROWS_AS(std::rethrow_exception(exception), std::runtime_error);
  }

  TEST_CASE("reception") {
    auto source = MessageClient();
    auto client = TmxIpMarketDataFeedClient(TmxIpConfiguration(), &source);
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=StockStatus\x1e" "57=20260920090000000");
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=MBXMessage\x1e" "5=AssignCOP\x1e" "55=ABX"
      "\x1e" "191=25.50\x1e" "57=20260920090000000");
    source.m_messages.push("\x01\x1e" "56=20260920090000000\x1c"
      "\x1e" "6=FutureMessage");
    flush_pending_routines();
    REQUIRE(source.get_count() == 3);
    REQUIRE_FALSE(client.is_finished());
    REQUIRE_FALSE(client.get_exception());
    client.close();
    REQUIRE(client.is_finished());
    REQUIRE(source.m_is_closed);
    REQUIRE_FALSE(client.get_exception());
  }
}
