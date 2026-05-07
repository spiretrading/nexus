#include <future>
#include <Beam/IO/LocalClientChannel.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkRecoveryClient.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OtcLinkRecoveryClient") {
  TEST_CASE("send_successful_snapshot_request") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("testid", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    auto client_task = std::async(std::launch::async, [&] {
      return client.request_snapshot(OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request = OtcLinkReplayRequestMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    REQUIRE(request.m_sender_comp_id == "testid");
    REQUIRE(request.m_appl_req_type ==
      OtcLinkReplayRequestMessage::Type::SNAPSHOT);
    REQUIRE(request.m_channel == OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT);
    reset(buffer);
    auto ack =
      OtcLinkReplayAckMessage::make_success("testid", request.m_appl_req_id,
        OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT, 0, 0);
    write(ack, out(buffer));
    channel->get_writer().write(buffer);
    auto response = client_task.get();
    REQUIRE(
      response.m_response == OtcLinkReplayAckMessage::ResponseType::SUCCESS);
    REQUIRE(response.m_text.empty());
  }

  TEST_CASE("send_snapshot_request_with_error_response") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("sender", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    auto client_task = std::async(std::launch::async, [&] {
      return client.request_snapshot(OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request = OtcLinkReplayRequestMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    reset(buffer);
    auto ack = OtcLinkReplayAckMessage::make_error(
      "sender", request.m_appl_req_id, OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT,
      OtcLinkReplayAckMessage::ResponseType::NOT_ENTITLED,
      std::string("User not authorized"));
    write(ack, out(buffer));
    channel->get_writer().write(buffer);
    auto response = client_task.get();
    REQUIRE(response.m_response ==
      OtcLinkReplayAckMessage::ResponseType::NOT_ENTITLED);
    REQUIRE(response.m_text == "User not authorized");
  }

  TEST_CASE("send_snapshot_request_with_limits_exceeded") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("client1", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    auto client_task = std::async(std::launch::async, [&] {
      return client.request_snapshot(OtcLinkChannelId::TRADE_REAL_TIME);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request = OtcLinkReplayRequestMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    reset(buffer);
    auto ack = OtcLinkReplayAckMessage::make_error(
      "client1", request.m_appl_req_id, OtcLinkChannelId::TRADE_REAL_TIME,
      OtcLinkReplayAckMessage::ResponseType::LIMITS_EXCEEDED,
      std::string("Rate limit exceeded"));
    write(ack, out(buffer));
    channel->get_writer().write(buffer);
    auto response = client_task.get();
    REQUIRE(response.m_response ==
      OtcLinkReplayAckMessage::ResponseType::LIMITS_EXCEEDED);
    REQUIRE(response.m_text == "Rate limit exceeded");
  }

  TEST_CASE("send_snapshot_request_connection_closed") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("closedid", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    auto client_task = std::async(std::launch::async, [&] {
      return client.request_snapshot(OtcLinkChannelId::QUOTE_BOOK_REAL_TIME);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    channel->get_connection().close();
    auto response = client_task.get();
    REQUIRE(response.m_response ==
      OtcLinkReplayAckMessage::ResponseType::MESSAGES_NOT_AVAILABLE);
    REQUIRE(response.m_text == "Connection closed.");
  }

  TEST_CASE("send_snapshot_request_error_without_text") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("notextid", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    auto client_task = std::async(std::launch::async, [&] {
      return client.request_snapshot(OtcLinkChannelId::QUOTE_INSIDE_REAL_TIME);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request = OtcLinkReplayRequestMessage::parse(
      std::string_view(buffer.get_data(), buffer.get_size()));
    reset(buffer);
    auto ack = OtcLinkReplayAckMessage::make_error(
      "notextid", request.m_appl_req_id,
      OtcLinkChannelId::QUOTE_INSIDE_REAL_TIME,
      OtcLinkReplayAckMessage::ResponseType::BAD_REQUEST);
    write(ack, out(buffer));
    channel->get_writer().write(buffer);
    auto response = client_task.get();
    REQUIRE(response.m_response ==
      OtcLinkReplayAckMessage::ResponseType::BAD_REQUEST);
    REQUIRE(response.m_text.empty());
  }

  TEST_CASE("multiple_snapshot_requests") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("multiid", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    for(auto i = 0; i < 3; ++i) {
      auto client_task = std::async(std::launch::async, [&] {
        return client.request_snapshot(OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT);
      });
      auto channel = server.accept();
      auto buffer = SharedBuffer();
      channel->get_reader().read(out(buffer));
      auto request = OtcLinkReplayRequestMessage::parse(
        std::string_view(buffer.get_data(), buffer.get_size()));
      REQUIRE(request.m_sender_comp_id == "multiid");
      reset(buffer);
      auto ack = OtcLinkReplayAckMessage::make_success(
        "multiid", request.m_appl_req_id,
        OtcLinkChannelId::QUOTE_BOOK_SNAPSHOT, 0, 0);
      write(ack, out(buffer));
      channel->get_writer().write(buffer);
      auto response = client_task.get();
      REQUIRE(response.m_response ==
        OtcLinkReplayAckMessage::ResponseType::SUCCESS);
    }
  }

  TEST_CASE("request_generates_unique_ids") {
    auto server = LocalServerConnection();
    auto client = OtcLinkRecoveryClient("uniqueid", [&] {
      return std::make_unique<LocalClientChannel>("otcm", server);
    });
    auto request_ids = std::vector<std::string>();
    for(auto i = 0; i < 5; ++i) {
      auto client_task = std::async(std::launch::async, [&] {
        return client.request_snapshot(OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT);
      });
      auto channel = server.accept();
      auto buffer = SharedBuffer();
      channel->get_reader().read(out(buffer));
      auto request = OtcLinkReplayRequestMessage::parse(
        std::string_view(buffer.get_data(), buffer.get_size()));
      REQUIRE(request.m_appl_req_id.size() == 8);
      request_ids.push_back(request.m_appl_req_id);
      reset(buffer);
      auto ack = OtcLinkReplayAckMessage::make_success(
        "uniqueid", request.m_appl_req_id,
        OtcLinkChannelId::QUOTE_INSIDE_SNAPSHOT, 0, 0);
      write(ack, out(buffer));
      channel->get_writer().write(buffer);
      client_task.get();
    }
    auto unique_ids = std::set(request_ids.begin(), request_ids.end());
    REQUIRE(unique_ids.size() == request_ids.size());
  }
}
