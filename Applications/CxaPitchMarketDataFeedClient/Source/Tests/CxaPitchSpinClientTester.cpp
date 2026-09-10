#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSpinClient.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  struct StubSession {
    std::vector<std::string> m_requests;
    std::shared_ptr<Queue<SharedBuffer>> m_messages;
    SharedBuffer m_payload;

    StubSession()
      : m_messages(std::make_shared<Queue<SharedBuffer>>()) {}

    CxaPitchMessage read() {
      m_payload = m_messages->pop();
      return CxaPitchMessage::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    template<typename M>
    void write(const M& message) {
      auto buffer = SharedBuffer();
      message.encode(out(buffer));
      m_requests.emplace_back(buffer.get_data(), buffer.get_size());
    }

    void close() {
      m_messages->close();
    }
  };

  using SpinClient = CxaPitchSpinClient<StubSession*>;

  void append_uint32(std::string& source, std::uint32_t value) {
    source += static_cast<char>(value & 0xFF);
    source += static_cast<char>((value >> 8) & 0xFF);
    source += static_cast<char>((value >> 16) & 0xFF);
    source += static_cast<char>((value >> 24) & 0xFF);
  }

  SharedBuffer encode_available(std::uint32_t sequence) {
    auto message = std::string();
    message += char(CxaPitchSpinImageAvailable::LENGTH);
    message += static_cast<char>(CxaPitchSpinImageAvailable::TYPE);
    append_uint32(message, sequence);
    return SharedBuffer(message.data(), message.size());
  }

  SharedBuffer encode_response(
      std::uint32_t sequence, std::uint32_t orders, char status) {
    auto message = std::string();
    message += char(CxaPitchSpinResponse::LENGTH);
    message += static_cast<char>(CxaPitchSpinResponse::TYPE);
    append_uint32(message, sequence);
    append_uint32(message, orders);
    message += status;
    return SharedBuffer(message.data(), message.size());
  }

  SharedBuffer encode_finished(std::uint32_t sequence) {
    auto message = std::string();
    message += char(CxaPitchSpinFinished::LENGTH);
    message += static_cast<char>(CxaPitchSpinFinished::TYPE);
    append_uint32(message, sequence);
    return SharedBuffer(message.data(), message.size());
  }

  SharedBuffer encode_order(std::uint8_t type) {
    static const auto SIDE_POSITION = std::size_t(18);
    auto length = CxaPitchAddOrder::LENGTH;
    if(type == CxaPitchTradingStatus::TYPE) {
      length = CxaPitchTradingStatus::LENGTH;
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
  TEST_CASE("read_offers") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_available(310169));
    session.m_messages->push(encode_available(310175));
    REQUIRE(client.get_offers()->pop() == 310169);
    REQUIRE(client.get_offers()->pop() == 310175);
    REQUIRE(client.get_progress() == 0);
  }

  TEST_CASE("request_snapshot") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_response(310175, 2, 'A'));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_order(CxaPitchTradingStatus::TYPE));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_finished(310175));
    auto spin = client.request(310175);
    REQUIRE(spin.m_sequence == 310175);
    REQUIRE(spin.m_status == CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(spin.m_messages.size() == 3);
    REQUIRE(client.get_progress() == 3);
    REQUIRE(CxaPitchMessage::parse(std::string_view(
      spin.m_messages[1].get_data(),
      spin.m_messages[1].get_size())).m_type == CxaPitchTradingStatus::TYPE);
    REQUIRE(session.m_requests.size() == 1);
    REQUIRE(session.m_requests[0] ==
      std::string_view("\x06\x81" "\x9f\xbb\x04\x00", 6));
  }

  TEST_CASE("reject_request") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_response(310175, 0, 'O'));
    auto spin = client.request(310175);
    REQUIRE(spin.m_status != CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(spin.m_messages.empty());
  }

  TEST_CASE("ignore_messages_outside_snapshot") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_response(310175, 1, 'A'));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_finished(310175));
    auto spin = client.request(310175);
    REQUIRE(spin.m_messages.size() == 1);
    REQUIRE(client.get_progress() == 1);
  }

  TEST_CASE("snapshot_progress") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_response(310175, 2, 'A'));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_available(310176));
    REQUIRE(client.get_offers()->pop() == 310176);
    REQUIRE(client.get_progress() == 1);
    session.m_messages->push(encode_available(310177));
    REQUIRE(client.get_offers()->pop() == 310177);
    REQUIRE(client.get_progress() == 1);
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_finished(310175));
    REQUIRE(client.request(310175).m_messages.size() == 2);
    REQUIRE(client.get_progress() == 2);
  }

  TEST_CASE("unexpected_snapshot_finish") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_finished(310174));
    session.m_messages->push(encode_response(310175, 1, 'A'));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_finished(310175));
    auto spin = client.request(310175);
    REQUIRE(spin.m_sequence == 310175);
    REQUIRE(spin.m_status == CxaPitchSpinResponse::ACCEPTED);
    REQUIRE(spin.m_messages.size() == 1);
  }

  TEST_CASE("incomplete_snapshot") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_response(310175, 2, 'A'));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_finished(310175));
    REQUIRE_THROWS_AS(client.request(310175), CxaPitchParserException);
  }

  TEST_CASE("oversized_snapshot") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_response(310175, 1, 'A'));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
    REQUIRE_THROWS_AS(client.request(310175), CxaPitchParserException);
  }

  TEST_CASE("malformed_snapshot_finish") {
    for(auto& finished :
        {SharedBuffer("\x02\x83", 2), encode_finished(310176)}) {
      auto session = StubSession();
      auto client = SpinClient(&session);
      session.m_messages->push(encode_response(310175, 1, 'A'));
      session.m_messages->push(encode_order(CxaPitchAddOrder::TYPE));
      session.m_messages->push(finished);
      REQUIRE_THROWS_AS(client.request(310175), CxaPitchParserException);
    }
  }

  TEST_CASE("malformed_snapshot_message") {
    auto session = StubSession();
    auto client = SpinClient(&session);
    session.m_messages->push(encode_response(310175, 1, 'A'));
    session.m_messages->push(SharedBuffer("\x02\x37", 2));
    flush_pending_routines();
    REQUIRE_THROWS_AS(client.request(310175), CxaPitchParserException);
    REQUIRE(client.get_progress() == 0);
  }
}
