#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpRecoveryClient.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  struct Fixture {
    using ProtocolClient =
      TmxIpProtocolClient<LocalClientChannel*, FixedTimeClient*>;
    using Client =
      TmxIpRecoveryClient<LocalClientChannel, ProtocolClient*, TriggerTimer*>;
    LocalServerConnection m_server;
    LocalServerConnection m_udp_server;
    Queue<std::stop_token> m_connections;
    std::unique_ptr<LocalClientChannel> m_udp_channel;
    std::unique_ptr<LocalServerChannel> m_udp_sender;
    FixedTimeClient m_time_client;
    std::unique_ptr<ProtocolClient> m_protocol_client;
    TriggerTimer m_timer;
    std::unique_ptr<Client> m_client;
    std::future<TmxIpRecoveryResult> m_result;

    Fixture()
      : Fixture(nullptr) {}

    explicit Fixture(std::function<std::shared_ptr<LocalClientChannel> (
        std::stop_token)> connection_builder)
        : m_time_client(boost::posix_time::time_from_string(
            "2026-09-20 12:00:00")) {
      auto sender = std::async(std::launch::async, [&] {
        return m_udp_server.accept();
      });
      m_udp_channel =
        std::make_unique<LocalClientChannel>("recovery", m_udp_server);
      m_udp_sender = sender.get();
      m_protocol_client =
        std::make_unique<ProtocolClient>(
          m_udp_channel.get(), &m_time_client);
      m_client = std::make_unique<Client>([=, this] (std::stop_token token) {
        m_connections.push(token);
        if(connection_builder) {
          return connection_builder(token);
        }
        return std::make_shared<LocalClientChannel>("request", m_server);
      }, m_protocol_client.get(), &m_timer);
    }

    ~Fixture() {
      m_server.close();
      m_client->close();
      if(m_result.valid()) {
        m_result.wait();
      }
    }

    std::unique_ptr<LocalServerChannel> start(
        const TmxIpRecoveryRequest& request) {
      m_result = std::async(std::launch::async, [=, this] {
        return m_client->request(request);
      });
      return m_server.accept();
    }

    void reject(const TmxIpRecoveryRequest& request) {
      while(m_connections.try_pop()) {}
      auto results = Queue<TmxIpRecoveryResult>();
      auto routine = RoutineHandler(spawn([&] {
        try {
          results.push(m_client->request(request));
        } catch(const std::exception&) {
          results.close(std::current_exception());
        }
      }));
      flush_pending_routines();
      auto connection = m_connections.try_pop();
      if(connection) {
        m_server.close();
      }
      routine.wait();
      REQUIRE_FALSE(connection.has_value());
      REQUIRE_THROWS_AS(results.pop(), IOException);
    }

    void publish(std::string_view payload, std::uint32_t sequence) {
      auto header_sequence = std::string(9, ' ');
      auto retransmission = ' ';
      if(sequence != 0) {
        header_sequence = std::format("{:09}", sequence);
        retransmission = '0';
      }
      auto packet = std::format("{}{:04}{}CDF{}0  T {}{}",
        TmxIpPacket::START, TmxIpHeader::LENGTH + payload.size(),
        header_sequence, retransmission, payload, TmxIpPacket::END);
      m_udp_sender->get_writer().write(from<SharedBuffer>(packet));
    }
  };

  std::string acknowledgement(std::uint32_t start, std::uint32_t end) {
    return std::format("ACK {:09}{:09}ACCEPTED{:99}{:22}", start, end, "", "");
  }
}

TEST_SUITE("TmxIpRecoveryClient") {
  TEST_CASE("recovery_header") {
    auto fixture = Fixture();
    fixture.m_client->reset(1);
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    server->get_writer().write(from<SharedBuffer>(acknowledgement(10, 11)));
    SUBCASE("missing") {}
    SUBCASE("mismatched") {
      fixture.publish("HDR  000000010000000012", 0);
    }
    SUBCASE("superseded") {
      fixture.publish("HDR  000000010000000011", 0);
      fixture.publish("HDR  000000010000000012", 0);
    }
    fixture.publish("stale", 10);
    fixture.publish("HDR  000000010000000011", 0);
    fixture.publish("current", 10);
    fixture.publish(std::format("TLR  000000002000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    auto session = std::uint64_t();
    REQUIRE(fixture.m_client->read(out(session)).m_payload == "current");
    REQUIRE(session == 1);
  }

  TEST_CASE("ordered_recovery_failure") {
    auto fixture = Fixture();
    fixture.m_client->reset(1);
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    server->get_writer().write(from<SharedBuffer>(acknowledgement(10, 11)));
    fixture.publish("HDR  000000010000000011", 0);
    fixture.publish("recovered", 10);
    fixture.publish(std::format("ERRORCANCELED{:100}", "canceled"), 0);
    REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    auto session = std::uint64_t();
    auto event = fixture.m_client->read_event(out(session));
    auto packet = std::get_if<TmxIpPacket>(&event);
    REQUIRE(packet);
    REQUIRE(packet->m_header.m_sequence == std::uint32_t(10));
    REQUIRE(packet->m_payload == "recovered");
    REQUIRE(session == 1);
    event = fixture.m_client->read_event(out(session));
    auto failure = std::get_if<TmxIpRecoveryFailure>(&event);
    REQUIRE(failure);
    REQUIRE(failure->m_request.m_start_sequence == 10);
    REQUIRE(failure->m_request.m_end_sequence == 11);
    REQUIRE(failure->m_session == 1);
    REQUIRE(failure->m_reason ==
      "recovery_failed TMX IP recovery error: canceled");
    REQUIRE(session == 1);
  }

  TEST_CASE("queued_session_packets") {
    auto fixture = Fixture();
    auto first = fixture.start(TmxIpRecoveryRequest(1, 1));
    first->get_writer().write(from<SharedBuffer>(acknowledgement(1, 1)));
    fixture.publish("HDR  000000001000000001", 0);
    fixture.publish("old", 1);
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    fixture.m_client->reset(1);
    fixture.reject(TmxIpRecoveryRequest(1, 1));
    auto next = fixture.start(TmxIpRecoveryRequest(2, 2));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(2, 2)));
    fixture.publish("HDR  000000001000000001", 0);
    fixture.publish("late", 1);
    fixture.publish("HDR  000000002000000002", 0);
    fixture.publish("late", 1);
    fixture.publish("new", 2);
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    auto session = std::uint64_t(99);
    REQUIRE(fixture.m_client->read(out(session)).m_payload == "old");
    REQUIRE(session == 0);
    REQUIRE(fixture.m_client->read(out(session)).m_payload == "new");
    REQUIRE(session == 1);
  }

  TEST_CASE("session_reset") {
    auto fixture = Fixture();
    auto first = fixture.start(TmxIpRecoveryRequest(1, 2));
    auto buffer = SharedBuffer();
    read_exact(first->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    SUBCASE("acknowledgement_pending") {}
    SUBCASE("delivery_pending") {
      first->get_writer().write(from<SharedBuffer>(acknowledgement(1, 2)));
      fixture.publish("HDR  000000001000000002", 0);
      fixture.publish("old", 1);
      auto session = std::uint64_t(99);
      REQUIRE(fixture.m_client->read(out(session)).m_payload == "old");
      REQUIRE(session == 0);
    }
    fixture.m_client->reset(1);
    REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    REQUIRE_THROWS_AS(
      fixture.m_client->request(TmxIpRecoveryRequest(1, 1), 0), IOException);
    fixture.reject(TmxIpRecoveryRequest(1, 1));
    fixture.reject(TmxIpRecoveryRequest(2, 3));
    auto next = fixture.start(TmxIpRecoveryRequest(3, 3));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(3, 3)));
    fixture.publish("HDR  000000001000000002", 0);
    fixture.publish("late", 1);
    fixture.publish("HDR  000000003000000003", 0);
    fixture.publish("late", 2);
    fixture.publish("new", 3);
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    auto session = std::uint64_t();
    REQUIRE(fixture.m_client->read(out(session)).m_payload == "new");
    REQUIRE(session == 1);
  }

  TEST_CASE("quarantined_ranges") {
    auto fixture = Fixture();
    auto first = fixture.start(TmxIpRecoveryRequest(10, 12));
    fixture.m_connections.pop();
    auto buffer = SharedBuffer();
    read_exact(first->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    SUBCASE("completed") {
      first->get_writer().write(from<SharedBuffer>(acknowledgement(10, 12)));
      fixture.publish("HDR  000000010000000012", 0);
      fixture.publish("first", 10);
      fixture.publish("second", 11);
      fixture.publish("third", 12);
      fixture.publish(std::format("TLR  000000003000000003{:100}", ""), 0);
      REQUIRE(fixture.m_result.get().m_sent_count == 3);
      REQUIRE(fixture.m_client->read().m_payload == "first");
      REQUIRE(fixture.m_client->read().m_payload == "second");
      REQUIRE(fixture.m_client->read().m_payload == "third");
      fixture.m_client->reset(1);
    }
    SUBCASE("partial") {
      first->get_writer().write(from<SharedBuffer>(acknowledgement(11, 12)));
      fixture.publish("HDR  000000011000000012", 0);
      fixture.publish("partial", 11);
      fixture.publish(std::format("TLR  000000003000000001{:100}", ""), 0);
      REQUIRE(fixture.m_result.wait_for(std::chrono::seconds(1)) ==
        std::future_status::ready);
      auto result = fixture.m_result.get();
      REQUIRE(result.m_requested_count == 3);
      REQUIRE(result.m_sent_count == 1);
      REQUIRE(fixture.m_client->read().m_payload == "partial");
      fixture.m_client->reset(1);
    }
    SUBCASE("deadline") {
      fixture.m_timer.trigger();
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    SUBCASE("error") {
      first->get_writer().write(from<SharedBuffer>(acknowledgement(10, 12)));
      fixture.publish("HDR  000000010000000012", 0);
      fixture.publish(std::format("ERRORCANCELED{:100}", "canceled"), 0);
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    for(auto& range : {TmxIpRecoveryRequest(10, 12),
        TmxIpRecoveryRequest(11, 11), TmxIpRecoveryRequest(9, 10),
        TmxIpRecoveryRequest(12, 13), TmxIpRecoveryRequest(9, 13)}) {
      fixture.reject(range);
    }
    auto connection = fixture.m_connections.try_pop();
    REQUIRE_FALSE(connection.has_value());
    fixture.m_client->reset(2);
    fixture.m_client->reset(3);
    fixture.reject(TmxIpRecoveryRequest(10, 12));
    connection = fixture.m_connections.try_pop();
    REQUIRE_FALSE(connection.has_value());
    auto next = fixture.start(TmxIpRecoveryRequest(13, 13));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(13, 13)));
    fixture.publish("HDR  000000010000000012", 0);
    fixture.publish("late", 12);
    fixture.publish("HDR  000000013000000013", 0);
    fixture.publish("late", 12);
    fixture.publish("current", 13);
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    auto session = std::uint64_t();
    REQUIRE(fixture.m_client->read(out(session)).m_payload == "current");
    REQUIRE(session == 3);
  }

  TEST_CASE("stale_transmission") {
    auto fixture = Fixture();
    auto first = fixture.start(TmxIpRecoveryRequest(10, 11));
    auto buffer = SharedBuffer();
    read_exact(first->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    fixture.m_timer.trigger();
    REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    auto next = fixture.start(TmxIpRecoveryRequest(12, 12));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(12, 12)));
    fixture.publish("HDR  000000010000000011", 0);
    fixture.publish(std::format("TLR  000000002000000002{:100}", "old"), 0);
    fixture.publish("HDR  000000012000000012", 0);
    fixture.publish("current", 12);
    fixture.publish(std::format("TLR  000000001000000001{:100}", "new"), 0);
    auto result = fixture.m_result.get();
    REQUIRE(result.m_description == "new");
    REQUIRE(result.m_requested_count == 1);
    REQUIRE(result.m_sent_count == 1);
    REQUIRE(fixture.m_client->read().m_payload == "current");
  }

  TEST_CASE("recovery") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    auto buffer = SharedBuffer();
    read_exact(server->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    auto request = std::string(buffer.get_data(), buffer.get_size());
    fixture.publish("HDR  000000010000000011", 0);
    fixture.publish("first", 10);
    auto response = acknowledgement(10, 11);
    server->get_writer().write(from<SharedBuffer>(response.substr(0, 17)));
    server->get_writer().write(from<SharedBuffer>(response.substr(17)));
    auto first = fixture.m_client->read();
    fixture.publish("second", 11);
    fixture.publish(std::format("TLR  000000002000000002{:100}", ""), 0);
    auto result = fixture.m_result.get();
    REQUIRE(request == "SEQN000000010000000011");
    REQUIRE(first.m_header.m_sequence == std::uint32_t(10));
    REQUIRE(first.m_header.m_service == "CDF");
    REQUIRE(first.m_payload == "first");
    auto second = fixture.m_client->read();
    REQUIRE(second.m_header.m_sequence == std::uint32_t(11));
    REQUIRE(second.m_header.m_service == "CDF");
    REQUIRE(second.m_payload == "second");
    REQUIRE(result.m_is_acknowledged);
    REQUIRE(result.m_status == TmxIpRecoveryResponse::Status::ACCEPTED);
    REQUIRE(result.m_start_sequence == 10);
    REQUIRE(result.m_end_sequence == 11);
    REQUIRE(result.m_requested_count == 2);
    REQUIRE(result.m_sent_count == 2);
    REQUIRE(result.m_description.empty());
  }
  TEST_CASE("acknowledgement") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    auto response = acknowledgement(0, 0);
    auto status = TmxIpRecoveryResponse::Status::ACCEPTED;
    auto is_acknowledged = true;
    auto description = std::string();
    SUBCASE("empty") {}
    SUBCASE("rejected") {
      response.replace(0, 4, "NACK");
      response.replace(22, 8, "REJECTED");
      response.replace(30, 6, "ERR004");
      status = TmxIpRecoveryResponse::Status::REJECTED;
      description = "ERR004";
      is_acknowledged = false;
    }
    server->get_writer().write(from<SharedBuffer>(response));
    auto result = fixture.m_result.get();
    REQUIRE(result.m_is_acknowledged == is_acknowledged);
    REQUIRE(result.m_status == status);
    REQUIRE(result.m_start_sequence == 0);
    REQUIRE(result.m_end_sequence == 0);
    REQUIRE(result.m_requested_count == 0);
    REQUIRE(result.m_sent_count == 0);
    REQUIRE(result.m_description == description);
    auto session = std::uint64_t(99);
    auto event = fixture.m_client->read_event(out(session));
    auto failure = std::get_if<TmxIpRecoveryFailure>(&event);
    REQUIRE(failure);
    REQUIRE(failure->m_request.m_start_sequence == 10);
    REQUIRE(failure->m_request.m_end_sequence == 11);
    REQUIRE(failure->m_session == 0);
    REQUIRE(failure->m_reason == "rejected " + description);
    REQUIRE(session == 0);
    auto next = fixture.start(TmxIpRecoveryRequest(12, 12));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    REQUIRE(fixture.m_result.get().m_is_acknowledged);
  }

  TEST_CASE("deadline") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    auto buffer = SharedBuffer();
    read_exact(server->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    auto is_failure = false;
    SUBCASE("acknowledgement") {}
    SUBCASE("delivery") {
      server->get_writer().write(from<SharedBuffer>(acknowledgement(10, 11)));
      fixture.publish("HDR  000000010000000011", 0);
      fixture.publish("first", 10);
      fixture.m_client->read();
    }
    SUBCASE("timer_failure") {
      is_failure = true;
    }
    if(is_failure) {
      fixture.m_timer.fail();
    } else {
      fixture.m_timer.trigger();
    }
    REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    auto next = fixture.start(TmxIpRecoveryRequest(12, 12));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    REQUIRE(fixture.m_result.get().m_is_acknowledged);
  }

  TEST_CASE("close") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    auto buffer = SharedBuffer();
    read_exact(server->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    SUBCASE("acknowledgement") {}
    SUBCASE("delivery") {
      server->get_writer().write(from<SharedBuffer>(acknowledgement(10, 11)));
      fixture.publish("HDR  000000010000000011", 0);
      fixture.publish("first", 10);
      fixture.m_client->read();
    }
    fixture.m_client->close();
    REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    REQUIRE_THROWS_AS(fixture.m_client->read(), std::exception);
    REQUIRE_THROWS_AS(
      fixture.m_client->request(TmxIpRecoveryRequest(12, 12)), IOException);
  }

  TEST_CASE("recovery_completion") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    server->get_writer().write(from<SharedBuffer>(acknowledgement(10, 11)));
    fixture.publish("HDR  000000010000000011", 0);
    SUBCASE("partial") {
      fixture.publish("outside", 9);
      fixture.publish("first", 10);
      fixture.publish(std::format("TLR  000000002000000001{:100}", "partial"),
        0);
      auto result = fixture.m_result.get();
      REQUIRE(result.m_requested_count == 2);
      REQUIRE(result.m_sent_count == 1);
      REQUIRE(result.m_description == "partial");
      REQUIRE(fixture.m_client->read().m_payload == "first");
    }
    SUBCASE("empty") {
      auto sent_count = std::uint32_t();
      SUBCASE("none_sent") {}
      SUBCASE("none_received") {
        sent_count = 2;
      }
      fixture.publish("outside", 9);
      fixture.publish("outside", 12);
      fixture.publish(std::format(
        "TLR  000000002{:09}{:100}", sent_count, "empty"), 0);
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
      auto session = std::uint64_t();
      auto event = fixture.m_client->read_event(out(session));
      auto failure = std::get_if<TmxIpRecoveryFailure>(&event);
      REQUIRE(failure);
      REQUIRE(failure->m_request.m_start_sequence == 10);
      REQUIRE(failure->m_request.m_end_sequence == 11);
      REQUIRE(session == 0);
      fixture.reject(TmxIpRecoveryRequest(10, 11));
    }
    SUBCASE("canceled") {
      fixture.publish(std::format("ERRORCANCELED{:100}", "canceled"), 0);
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    SUBCASE("failed") {
      fixture.publish(std::format("ERRORFAILED  {:100}", "failed"), 0);
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    auto next = fixture.start(TmxIpRecoveryRequest(12, 12));
    next->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    REQUIRE(fixture.m_result.get().m_is_acknowledged);
  }

  TEST_CASE("request_limit") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_client->get_maximum_count() == 10000);
    fixture.publish(
      "HBEAT[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
      "TDOTDR  00.1000000002", 0);
    flush_pending_routines();
    REQUIRE(fixture.m_client->get_maximum_count() == 2);
    REQUIRE_THROWS_AS(
      fixture.m_client->request(TmxIpRecoveryRequest(1, 3)), IOException);
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    fixture.publish(
      "HBEAT[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
      "TDOTDR  00.1000000001", 0);
    server->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    REQUIRE(fixture.m_result.get().m_is_acknowledged);
    flush_pending_routines();
    REQUIRE(fixture.m_client->get_maximum_count() == 1);
    REQUIRE_THROWS_AS(
      fixture.m_client->request(TmxIpRecoveryRequest(10, 11)), IOException);
  }

  TEST_CASE("serialized_requests") {
    auto fixture = Fixture();
    auto first = fixture.start(TmxIpRecoveryRequest(10, 10));
    fixture.m_connections.pop();
    first->get_writer().write(from<SharedBuffer>(acknowledgement(10, 10)));
    fixture.publish("HDR  000000010000000010", 0);
    fixture.publish("first", 10);
    fixture.m_client->read();
    auto results = Queue<TmxIpRecoveryResult>();
    auto request = RoutineHandler(spawn([&] {
      try {
        results.push(fixture.m_client->request(TmxIpRecoveryRequest(11, 11)));
      } catch(const std::exception&) {
        results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    auto connection = fixture.m_connections.try_pop();
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    auto first_result = fixture.m_result.get();
    auto second = fixture.m_server.accept();
    second->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    auto second_result = results.pop();
    request.wait();
    REQUIRE_FALSE(connection.has_value());
    REQUIRE(first_result.m_sent_count == 1);
    REQUIRE(second_result.m_is_acknowledged);
  }

  TEST_CASE("queued_request_limit") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(1, 1));
    fixture.m_connections.pop();
    server->get_writer().write(from<SharedBuffer>(acknowledgement(1, 1)));
    fixture.publish("HDR  000000001000000001", 0);
    fixture.publish("first", 1);
    fixture.m_client->read();
    auto results = Queue<TmxIpRecoveryResult>();
    auto request = RoutineHandler(spawn([&] {
      try {
        results.push(fixture.m_client->request(TmxIpRecoveryRequest(2, 3)));
      } catch(const std::exception&) {
        results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    fixture.publish(
      "HBEAT[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
      "TDOTDR  00.1000000001", 0);
    flush_pending_routines();
    auto limit = fixture.m_client->get_maximum_count();
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    auto first_result = fixture.m_result.get();
    flush_pending_routines();
    auto connection = fixture.m_connections.try_pop();
    fixture.m_server.close();
    request.wait();
    REQUIRE(limit == 1);
    REQUIRE(first_result.m_sent_count == 1);
    REQUIRE_FALSE(connection.has_value());
    REQUIRE_THROWS_AS(results.pop(), IOException);
  }

  TEST_CASE("transport_failure") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 11));
    auto buffer = SharedBuffer();
    read_exact(server->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
    auto is_udp_failure = false;
    SUBCASE("tcp") {
      server->get_connection().close();
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    SUBCASE("malformed_acknowledgement") {
      server->get_writer().write(from<SharedBuffer>(
        std::string(TmxIpRecoveryResponse::LENGTH, 'x')));
      REQUIRE_THROWS_AS(fixture.m_result.get(), TmxIpParserException);
    }
    SUBCASE("udp") {
      fixture.m_udp_sender->get_connection().close();
      is_udp_failure = true;
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    if(is_udp_failure) {
      REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
      REQUIRE_THROWS_AS(
        fixture.m_client->request(TmxIpRecoveryRequest(12, 12)), IOException);
    } else {
      auto next = fixture.start(TmxIpRecoveryRequest(12, 12));
      next->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
      REQUIRE(fixture.m_result.get().m_is_acknowledged);
    }
  }

  TEST_CASE("malformed_datagram") {
    auto fixture = Fixture();
    auto server = fixture.start(TmxIpRecoveryRequest(10, 10));
    server->get_writer().write(from<SharedBuffer>(acknowledgement(10, 10)));
    fixture.publish("HDR  000000010000000010", 0);
    fixture.m_udp_sender->get_writer().write(from<SharedBuffer>("invalid"));
    fixture.publish("first", 10);
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    REQUIRE(fixture.m_client->read().m_payload == "first");
  }

  TEST_CASE("late_connection") {
    auto server = LocalServerConnection();
    auto gate = Queue<bool>();
    auto connected = Queue<bool>();
    auto fixture = Fixture([&] (std::stop_token) {
      auto channel = std::make_shared<LocalClientChannel>("request", server);
      connected.push(true);
      gate.pop();
      return channel;
    });
    fixture.m_result = std::async(std::launch::async, [&] {
      return fixture.m_client->request(TmxIpRecoveryRequest(1, 1));
    });
    auto channel = server.accept();
    connected.pop();
    fixture.m_timer.trigger();
    flush_pending_routines();
    gate.push(true);
    REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(channel->get_reader().read(out(buffer)), IOException);
    fixture.m_result = std::async(std::launch::async, [&] {
      return fixture.m_client->request(TmxIpRecoveryRequest(2, 2));
    });
    auto next = server.accept();
    gate.push(true);
    next->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    REQUIRE(fixture.m_result.get().m_is_acknowledged);
  }

  TEST_CASE("connection_cancellation") {
    auto gate = Queue<bool>();
    auto fixture = Fixture([&] (std::stop_token token) ->
        std::shared_ptr<LocalClientChannel> {
      auto cancellation = std::stop_callback(token, [&] {
        gate.close();
      });
      gate.pop();
      return {};
    });
    fixture.m_result = std::async(std::launch::async, [&] {
      return fixture.m_client->request(TmxIpRecoveryRequest(1, 1));
    });
    auto token = fixture.m_connections.pop();
    SUBCASE("deadline") {
      fixture.m_timer.trigger();
    }
    SUBCASE("close") {
      fixture.m_client->close();
    }
    REQUIRE_THROWS_AS(fixture.m_result.get(), std::exception);
    REQUIRE(token.stop_requested());
  }

  TEST_CASE("cancellation_barrier") {
    auto fixture = Fixture();
    auto first_results = Queue<TmxIpRecoveryResult>();
    auto first_request = RoutineHandler(spawn([&] {
      try {
        first_results.push(
          fixture.m_client->request(TmxIpRecoveryRequest(1, 1)));
      } catch(const std::exception&) {
        first_results.close(std::current_exception());
      }
    }));
    auto first = fixture.m_server.accept();
    auto token = fixture.m_connections.pop();
    first->get_writer().write(from<SharedBuffer>(acknowledgement(1, 1)));
    fixture.publish("HDR  000000001000000001", 0);
    fixture.publish("first", 1);
    fixture.m_client->read();
    auto gate = Queue<bool>();
    auto notifications = Queue<bool>();
    auto callback = std::stop_callback(token, [&] {
      notifications.push(true);
      gate.pop();
    });
    fixture.m_timer.trigger();
    notifications.pop();
    flush_pending_routines();
    auto is_complete = first_results.is_broken();
    auto second_results = Queue<TmxIpRecoveryResult>();
    auto second_request = RoutineHandler(spawn([&] {
      try {
        second_results.push(
          fixture.m_client->request(TmxIpRecoveryRequest(2, 2)));
      } catch(const std::exception&) {
        second_results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    auto connection = fixture.m_connections.try_pop();
    gate.push(true);
    first_request.wait();
    auto second = fixture.m_server.accept();
    second->get_writer().write(from<SharedBuffer>(acknowledgement(0, 0)));
    auto result = second_results.pop();
    second_request.wait();
    REQUIRE_FALSE(is_complete);
    REQUIRE_FALSE(connection.has_value());
    REQUIRE_THROWS_AS(first_results.pop(), IOException);
    REQUIRE(result.m_is_acknowledged);
  }

  TEST_CASE("consecutive_deadlines") {
    auto fixture = Fixture();
    for(auto sequence = std::uint32_t(1); sequence != 3; ++sequence) {
      auto server = fixture.start(TmxIpRecoveryRequest(sequence, sequence));
      auto buffer = SharedBuffer();
      read_exact(
        server->get_reader(), out(buffer), TmxIpRecoveryRequest::LENGTH);
      fixture.m_timer.trigger();
      REQUIRE_THROWS_AS(fixture.m_result.get(), IOException);
    }
    auto server = fixture.start(TmxIpRecoveryRequest(3, 3));
    server->get_writer().write(from<SharedBuffer>(acknowledgement(3, 3)));
    fixture.publish("HDR  000000003000000003", 0);
    fixture.publish("recovered", 3);
    fixture.publish(std::format("TLR  000000001000000001{:100}", ""), 0);
    REQUIRE(fixture.m_result.get().m_sent_count == 1);
    REQUIRE(fixture.m_client->read().m_payload == "recovered");
  }

}
