#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSpinClient.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  constexpr auto SIDE_POSITION =
    CxaPitchMessage::HEADER_LENGTH + 2 * sizeof(std::uint64_t);

  SharedBuffer encode(const IsCxaPitchSessionMessage auto& message) {
    auto buffer = SharedBuffer();
    message.encode(out(buffer));
    return buffer;
  }

  struct StubSession {
    Queue<SharedBuffer> m_requests;
    Queue<SharedBuffer> m_messages;
    SharedBuffer m_payload;

    CxaPitchMessage read() {
      auto buffer = m_messages.pop();
      reset(m_payload);
      append(m_payload, buffer);
      return CxaPitchMessage::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void write(const IsCxaPitchSessionMessage auto& message) {
      m_requests.push(encode(message));
    }

    void close() {
      m_messages.close();
    }
  };

  using SpinClient = CxaPitchSpinClient<StubSession*>;

  struct Fixture {
    StubSession m_session;
    SpinClient m_client;
    Queue<Expect<CxaPitchSnapshot>> m_results;
    RoutineHandler m_request;

    Fixture()
      : m_client(&m_session) {}

    ~Fixture() {
      m_client.close();
      m_request.wait();
    }

    void request(std::uint32_t sequence) {
      m_request = spawn([=, this] {
        m_results.push(try_call([&] {
          return m_client.load_snapshot(sequence);
        }));
      });
      flush_pending_routines();
      auto request = m_session.m_requests.try_pop();
      REQUIRE(request.has_value());
      REQUIRE(*request == encode(CxaPitchSpinRequest(sequence)));
      REQUIRE(!m_results.try_pop());
    }

    Expect<CxaPitchSnapshot> read() {
      flush_pending_routines();
      auto result = m_results.try_pop();
      REQUIRE(result.has_value());
      m_request.wait();
      return *result;
    }

    void send(const IsCxaPitchSessionMessage auto& message) {
      send(encode(message));
    }

    void send(const SharedBuffer& message) {
      m_session.m_messages.push(message);
    }
  };

  SharedBuffer encode_message(std::uint8_t type) {
    auto length = CxaPitchAddOrder::LENGTH;
    if(type == CxaPitchTradingStatus::TYPE) {
      length = CxaPitchTradingStatus::LENGTH;
    } else if(type == CxaPitchCalculatedValue::TYPE) {
      length = CxaPitchCalculatedValue::LENGTH;
    } else if(type == CxaPitchAuctionUpdate::TYPE) {
      length = CxaPitchAuctionUpdate::LENGTH;
    }
    auto message = std::string(length, char(0));
    message[0] = static_cast<char>(length);
    message[1] = static_cast<char>(type);
    if(type == CxaPitchAddOrder::TYPE) {
      message[SIDE_POSITION] = 'B';
    }
    return SharedBuffer(message.data(), message.size());
  }
}

TEST_SUITE("CxaPitchSpinClient") {
  TEST_CASE("snapshot_sequences") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    auto sequences = std::make_shared<Queue<std::uint32_t>>();
    client.monitor_snapshot_sequences(sequences);
    REQUIRE(!sequences->try_pop());
    session.m_messages.push(encode(CxaPitchSpinImageAvailable(310169)));
    session.m_messages.push(encode(CxaPitchSpinImageAvailable(310175)));
    flush_pending_routines();
    auto offer = sequences->try_pop();
    REQUIRE(offer.has_value());
    REQUIRE(*offer == 310169);
    offer = sequences->try_pop();
    REQUIRE(offer.has_value());
    REQUIRE(*offer == 310175);
    REQUIRE(!sequences->try_pop());
  }

  TEST_CASE("snapshot_sequence_monitors") {
    auto fixture = Fixture();
    auto first = std::make_shared<Queue<std::uint32_t>>();
    auto second = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(first);
    fixture.m_client.monitor_snapshot_sequences(second);
    fixture.send(CxaPitchSpinImageAvailable(310175));
    flush_pending_routines();
    auto sequence = first->try_pop();
    REQUIRE(sequence.has_value());
    REQUIRE(*sequence == 310175);
    sequence = second->try_pop();
    REQUIRE(sequence.has_value());
    REQUIRE(*sequence == 310175);
    first->close();
    fixture.send(CxaPitchSpinImageAvailable(310176));
    flush_pending_routines();
    sequence = second->try_pop();
    REQUIRE(sequence.has_value());
    REQUIRE(*sequence == 310176);
    REQUIRE(!second->is_broken());
    REQUIRE(!first->try_pop());
  }

  TEST_CASE("latest_snapshot_sequence") {
    auto fixture = Fixture();
    fixture.send(CxaPitchSpinImageAvailable(310169));
    fixture.send(CxaPitchSpinImageAvailable(310175));
    flush_pending_routines();
    auto sequences = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(sequences);
    auto sequence = sequences->try_pop();
    REQUIRE(sequence.has_value());
    REQUIRE(*sequence == 310175);
    REQUIRE(!sequences->try_pop());
    fixture.send(CxaPitchSpinImageAvailable(310176));
    flush_pending_routines();
    sequence = sequences->try_pop();
    REQUIRE(sequence.has_value());
    REQUIRE(*sequence == 310176);
    REQUIRE(!sequences->try_pop());
  }

  TEST_CASE("snapshot_sequence_monitor_failure") {
    auto fixture = Fixture();
    auto first = std::make_shared<Queue<std::uint32_t>>();
    auto second = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(first);
    fixture.m_client.monitor_snapshot_sequences(second);
    fixture.send(
      SharedBuffer("\x02\x80", CxaPitchMessage::HEADER_LENGTH));
    flush_pending_routines();
    REQUIRE(first->is_broken());
    REQUIRE(second->is_broken());
    REQUIRE_THROWS_AS(first->pop(), CxaPitchParserException);
    REQUIRE_THROWS_AS(second->pop(), CxaPitchParserException);
    auto late = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(late);
    REQUIRE(late->is_broken());
    REQUIRE_THROWS_AS(late->pop(), CxaPitchParserException);
  }

  TEST_CASE("snapshot") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 2, 'A'));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(
      encode_message(CxaPitchTradingStatus::TYPE));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(CxaPitchSpinFinished(310175));
    auto snapshot = fixture.read().get();
    REQUIRE(snapshot.m_sequence == 310175);
    REQUIRE(snapshot.m_status == CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(snapshot.m_messages.size() == 3);
    REQUIRE(CxaPitchMessage::parse(std::string_view(
      snapshot.m_messages[1].get_data(),
      snapshot.m_messages[1].get_size())).m_type ==
        CxaPitchTradingStatus::TYPE);
    REQUIRE(!fixture.m_session.m_requests.try_pop());
  }

  TEST_CASE("zero_order_snapshot") {
    auto fixture = Fixture();
    auto messages = std::vector<SharedBuffer>();
    SUBCASE("empty") {}
    SUBCASE("symbol_messages") {
      messages.push_back(encode_message(CxaPitchTradingStatus::TYPE));
      messages.push_back(encode_message(CxaPitchCalculatedValue::TYPE));
      messages.push_back(encode_message(CxaPitchAuctionUpdate::TYPE));
    }
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 0, 'A'));
    for(const auto& message : messages) {
      fixture.send(message);
    }
    flush_pending_routines();
    REQUIRE(!fixture.m_results.try_pop());
    fixture.send(CxaPitchSpinFinished(310175));
    auto snapshot = fixture.read().get();
    REQUIRE(snapshot.m_sequence == 310175);
    REQUIRE(snapshot.m_status == CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(snapshot.m_messages == messages);
  }

  TEST_CASE("newer_snapshot") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310180, 1, 'A'));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(CxaPitchSpinFinished(310180));
    auto snapshot = fixture.read().get();
    REQUIRE(snapshot.m_sequence == 310180);
    REQUIRE(snapshot.m_status == CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(snapshot.m_messages.size() == 1);
  }

  TEST_CASE("rejected_request") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 0, 'O'));
    auto snapshot = fixture.read().get();
    REQUIRE(snapshot.m_sequence == 310175);
    REQUIRE(snapshot.m_status == 'O');
    REQUIRE(snapshot.m_messages.empty());
    fixture.request(310176);
    fixture.send(CxaPitchSpinResponse(310176, 1, 'A'));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(CxaPitchSpinFinished(310176));
    snapshot = fixture.read().get();
    REQUIRE(snapshot.m_sequence == 310176);
    REQUIRE(snapshot.m_status == CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(snapshot.m_messages.size() == 1);
  }

  TEST_CASE("snapshot_storage") {
    auto fixture = Fixture();
    auto messages = std::vector{encode_message(CxaPitchAddOrder::TYPE),
      encode_message(CxaPitchTradingStatus::TYPE),
      encode_message(CxaPitchCalculatedValue::TYPE),
      encode_message(CxaPitchAuctionUpdate::TYPE)};
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 1, 'A'));
    for(const auto& message : messages) {
      fixture.send(message);
    }
    fixture.send(CxaPitchSpinFinished(310175));
    auto first = fixture.read().get();
    fixture.request(310180);
    auto replacement = encode_message(CxaPitchAddOrder::TYPE);
    replacement.get_mutable_data()[CxaPitchMessage::HEADER_LENGTH] = 'X';
    fixture.send(CxaPitchSpinResponse(310180, 1, 'A'));
    fixture.send(replacement);
    fixture.send(CxaPitchSpinFinished(310180));
    auto second = fixture.read().get();
    fixture.m_client.close();
    REQUIRE(first.m_sequence == 310175);
    REQUIRE(first.m_messages == messages);
    REQUIRE(second.m_sequence == 310180);
    REQUIRE(second.m_messages.size() == 1);
    REQUIRE(second.m_messages.front() == replacement);
  }

  TEST_CASE("offers_during_snapshot") {
    auto fixture = Fixture();
    auto sequences = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(sequences);
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 2, 'A'));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    flush_pending_routines();
    REQUIRE(!fixture.m_results.try_pop());
    fixture.send(CxaPitchSpinImageAvailable(310176));
    fixture.send(CxaPitchSpinImageAvailable(310177));
    flush_pending_routines();
    auto offer = sequences->try_pop();
    REQUIRE(offer.has_value());
    REQUIRE(*offer == 310176);
    offer = sequences->try_pop();
    REQUIRE(offer.has_value());
    REQUIRE(*offer == 310177);
    REQUIRE(!fixture.m_results.try_pop());
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(CxaPitchSpinFinished(310175));
    REQUIRE(fixture.read().get().m_messages.size() == 2);
  }

  TEST_CASE("request_write_failure") {
    auto fixture = Fixture();
    auto exception = std::make_exception_ptr(
      IOException("Snapshot request write failed."));
    fixture.m_session.m_requests.close(exception);
    fixture.m_request = spawn([&] {
      fixture.m_results.push(try_call([&] {
        return fixture.m_client.load_snapshot(310175);
      }));
    });
    auto result = fixture.read();
    REQUIRE(result.is_exception());
    REQUIRE_THROWS_WITH_AS(
      result.get(), "Snapshot request write failed.", IOException);
  }

  TEST_CASE("session_disconnect") {
    auto fixture = Fixture();
    auto sequences = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(sequences);
    fixture.request(310175);
    SUBCASE("pending_response") {}
    SUBCASE("partial_snapshot") {
      fixture.send(CxaPitchSpinResponse(310175, 2, 'A'));
      fixture.send(encode_message(CxaPitchAddOrder::TYPE));
      flush_pending_routines();
      REQUIRE(!fixture.m_results.try_pop());
    }
    fixture.m_session.m_messages.close(std::make_exception_ptr(
      EndOfFileException("Snapshot session disconnected.")));
    auto result = fixture.read();
    REQUIRE(result.is_exception());
    REQUIRE_THROWS_WITH_AS(
      result.get(), "Snapshot session disconnected.", EndOfFileException);
    REQUIRE(sequences->is_broken());
    REQUIRE_THROWS_WITH_AS(
      sequences->pop(), "Snapshot session disconnected.", EndOfFileException);
  }

  TEST_CASE("close_pending_request") {
    auto fixture = Fixture();
    auto sequences = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(sequences);
    fixture.request(310175);
    SUBCASE("pending_response") {}
    SUBCASE("pending_finish") {
      fixture.send(CxaPitchSpinResponse(310175, 2, 'A'));
      fixture.send(encode_message(CxaPitchAddOrder::TYPE));
      flush_pending_routines();
      REQUIRE(!fixture.m_results.try_pop());
    }
    fixture.m_client.close();
    REQUIRE_THROWS_AS(fixture.read().get(), PipeBrokenException);
    REQUIRE(sequences->is_broken());
    REQUIRE_THROWS_AS(sequences->pop(), PipeBrokenException);
    auto late = std::make_shared<Queue<std::uint32_t>>();
    fixture.m_client.monitor_snapshot_sequences(late);
    REQUIRE(late->is_broken());
    REQUIRE_THROWS_AS(late->pop(), PipeBrokenException);
  }

  TEST_CASE("incomplete_snapshot") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 2, 'A'));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(CxaPitchSpinFinished(310175));
    REQUIRE_THROWS_AS(fixture.read().get(), CxaPitchParserException);
  }

  TEST_CASE("oversized_snapshot") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 1, 'A'));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    fixture.send(encode_message(CxaPitchAddOrder::TYPE));
    REQUIRE_THROWS_AS(fixture.read().get(), CxaPitchParserException);
  }

  TEST_CASE("malformed_snapshot_response") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(
      SharedBuffer("\x02\x82", CxaPitchMessage::HEADER_LENGTH));
    REQUIRE_THROWS_AS(fixture.read().get(), CxaPitchParserException);
  }

  TEST_CASE("malformed_snapshot_finish") {
    for(const auto& finished : {SharedBuffer("\x02\x83",
        CxaPitchMessage::HEADER_LENGTH),
        encode(CxaPitchSpinFinished(310176))}) {
      auto fixture = Fixture();
      fixture.request(310175);
      fixture.send(CxaPitchSpinResponse(310175, 1, 'A'));
      fixture.send(encode_message(CxaPitchAddOrder::TYPE));
      fixture.send(finished);
      REQUIRE_THROWS_AS(fixture.read().get(), CxaPitchParserException);
    }
  }

  TEST_CASE("malformed_snapshot_message") {
    auto fixture = Fixture();
    fixture.request(310175);
    fixture.send(CxaPitchSpinResponse(310175, 1, 'A'));
    auto message = SharedBuffer();
    SUBCASE("truncated") {
      message = SharedBuffer("\x02\x37", CxaPitchMessage::HEADER_LENGTH);
    }
    SUBCASE("invalid_side") {
      message = encode_message(CxaPitchAddOrder::TYPE);
      message.get_mutable_data()[SIDE_POSITION] = 'X';
    }
    fixture.send(message);
    REQUIRE_THROWS_AS(fixture.read().get(), CxaPitchParserException);
  }
}
