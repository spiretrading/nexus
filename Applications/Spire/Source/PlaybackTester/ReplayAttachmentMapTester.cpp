#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Playback/ReplayAttachmentMap.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/PropertyHubMember.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Spire;

namespace {
  const auto BASE = time_from_string("2024-06-01 10:00:00");

  struct Fixture {
    FixedTimeClient m_time_client;
    std::shared_ptr<ReplayAttachmentMap> m_attachment_map;
    std::shared_ptr<ReplaySession> m_first;
    std::shared_ptr<ReplaySession> m_second;

    Fixture()
      : m_time_client(BASE + hours(1)),
        m_attachment_map(std::make_shared<ReplayAttachmentMap>()),
        m_first(make_session()),
        m_second(make_session()) {}

    std::shared_ptr<ReplaySession> make_session() {
      return std::make_shared<ReplaySession>(Timeline(BASE, hours(2)),
        minutes(10), Decimal(1), TimeClient(&m_time_client));
    }
  };

  struct Update {
    std::shared_ptr<PropertyHub> m_hub;
    std::shared_ptr<ReplaySession> m_session;
  };
}

TEST_SUITE("ReplayAttachmentMap") {
  TEST_CASE("attach_and_detach") {
    run_test([] {
      auto fixture = Fixture();
      auto& attachment_map = *fixture.m_attachment_map;
      auto updates = std::vector<Update>();
      auto connection = attachment_map.connect_update_signal(
        [&] (const auto& updated_hub, const auto& session) {
          updates.push_back(Update(updated_hub, session));
        });
      auto hub = std::make_shared<PropertyHub>();
      REQUIRE(!attachment_map.find(hub));
      attachment_map.attach(hub, fixture.m_first);
      REQUIRE(attachment_map.find(hub) == fixture.m_first);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.back().m_hub == hub);
      REQUIRE(updates.back().m_session == fixture.m_first);
      attachment_map.attach(hub, fixture.m_first);
      REQUIRE(updates.size() == 1);
      attachment_map.detach(hub);
      REQUIRE(!attachment_map.find(hub));
      REQUIRE(updates.size() == 2);
      REQUIRE(updates.back().m_hub == hub);
      REQUIRE(!updates.back().m_session);
      attachment_map.detach(hub);
      REQUIRE(updates.size() == 2);
    });
  }

  TEST_CASE("attaching_to_another_session_moves_the_hub") {
    run_test([] {
      auto fixture = Fixture();
      auto& attachment_map = *fixture.m_attachment_map;
      auto hub = std::make_shared<PropertyHub>();
      attachment_map.attach(hub, fixture.m_first);
      auto updates = std::vector<Update>();
      auto connection = attachment_map.connect_update_signal(
        [&] (const auto& updated_hub, const auto& session) {
          updates.push_back(Update(updated_hub, session));
        });
      attachment_map.attach(hub, fixture.m_second);
      REQUIRE(attachment_map.find(hub) == fixture.m_second);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.back().m_hub == hub);
      REQUIRE(updates.back().m_session == fixture.m_second);
      REQUIRE(attachment_map.get_hubs(*fixture.m_first).empty());
      REQUIRE(attachment_map.get_hubs(*fixture.m_second) ==
        std::vector<std::shared_ptr<PropertyHub>>{hub});
    });
  }

  TEST_CASE("attaching_to_a_null_session_detaches") {
    run_test([] {
      auto fixture = Fixture();
      auto& attachment_map = *fixture.m_attachment_map;
      auto hub = std::make_shared<PropertyHub>();
      attachment_map.attach(hub, fixture.m_first);
      attachment_map.attach(hub, nullptr);
      REQUIRE(!attachment_map.find(hub));
      REQUIRE(attachment_map.get_hubs(*fixture.m_first).empty());
    });
  }

  TEST_CASE("attached_hub_list") {
    run_test([] {
      auto fixture = Fixture();
      auto& attachment_map = *fixture.m_attachment_map;
      auto first_hub = std::make_shared<PropertyHub>();
      attachment_map.attach(first_hub, fixture.m_first);
      auto list =
        make_attached_hub_list(fixture.m_attachment_map, fixture.m_first);
      REQUIRE(list->get_size() == 1);
      REQUIRE(list->get(0) == first_hub);
      auto second_hub = std::make_shared<PropertyHub>();
      REQUIRE(list->push(second_hub) == QValidator::State::Acceptable);
      REQUIRE(list->get_size() == 2);
      REQUIRE(list->get(1) == second_hub);
      REQUIRE(attachment_map.find(second_hub) == fixture.m_first);
      REQUIRE(list->push(second_hub) == QValidator::State::Acceptable);
      REQUIRE(list->get_size() == 2);
      REQUIRE(list->remove(0) == QValidator::State::Acceptable);
      REQUIRE(list->get_size() == 1);
      REQUIRE(list->get(0) == second_hub);
      REQUIRE(!attachment_map.find(first_hub));
      attachment_map.detach(second_hub);
      REQUIRE(list->get_size() == 0);
    });
  }

  TEST_CASE("attached_hub_list_loses_a_hub_attached_elsewhere") {
    run_test([] {
      auto fixture = Fixture();
      auto first_list =
        make_attached_hub_list(fixture.m_attachment_map, fixture.m_first);
      auto second_list =
        make_attached_hub_list(fixture.m_attachment_map, fixture.m_second);
      auto hub = std::make_shared<PropertyHub>();
      first_list->push(hub);
      auto added = 0;
      auto removed = std::vector<std::shared_ptr<PropertyHub>>();
      auto first_connection = first_list->connect_operation_signal(
        [&] (const PropertyHubListModel::Operation& operation) {
          visit(operation,
            [&] (const PropertyHubListModel::PreRemoveOperation& operation) {
              removed.push_back(first_list->get(operation.m_index));
            });
        });
      auto second_connection = second_list->connect_operation_signal(
        [&] (const PropertyHubListModel::Operation& operation) {
          visit(operation,
            [&] (const PropertyHubListModel::AddOperation& operation) {
              ++added;
            });
        });
      second_list->push(hub);
      REQUIRE(first_list->get_size() == 0);
      REQUIRE(removed == std::vector<std::shared_ptr<PropertyHub>>{hub});
      REQUIRE(second_list->get_size() == 1);
      REQUIRE(second_list->get(0) == hub);
      REQUIRE(added == 1);
      REQUIRE(fixture.m_attachment_map->find(hub) == fixture.m_second);
    });
  }

  TEST_CASE("clearing_an_attached_hub_list_detaches_its_hubs") {
    run_test([] {
      auto fixture = Fixture();
      auto& attachment_map = *fixture.m_attachment_map;
      auto list =
        make_attached_hub_list(fixture.m_attachment_map, fixture.m_first);
      auto first_hub = std::make_shared<PropertyHub>();
      auto second_hub = std::make_shared<PropertyHub>();
      auto third_hub = std::make_shared<PropertyHub>();
      list->push(first_hub);
      list->push(second_hub);
      attachment_map.attach(third_hub, fixture.m_second);
      clear(*list);
      REQUIRE(list->get_size() == 0);
      REQUIRE(!attachment_map.find(first_hub));
      REQUIRE(!attachment_map.find(second_hub));
      REQUIRE(attachment_map.find(third_hub) == fixture.m_second);
    });
  }

  TEST_CASE("attached_session_model") {
    run_test([] {
      auto fixture = Fixture();
      auto& attachment_map = *fixture.m_attachment_map;
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      auto hub = std::make_shared<PropertyHub>();
      auto member = PropertyHubMember(roster, widget, "", hub);
      auto attached =
        make_attached_session_model(member, fixture.m_attachment_map);
      auto updates = std::vector<std::shared_ptr<ReplaySession>>();
      auto connection = attached->connect_update_signal(
        [&] (const auto& session) {
          updates.push_back(session);
        });
      REQUIRE(!attached->get());
      attachment_map.attach(std::make_shared<PropertyHub>(), fixture.m_first);
      REQUIRE(!attached->get());
      REQUIRE(updates.empty());
      attachment_map.attach(hub, fixture.m_first);
      REQUIRE(attached->get() == fixture.m_first);
      REQUIRE(updates.size() == 1);
      attachment_map.attach(hub, fixture.m_second);
      REQUIRE(attached->get() == fixture.m_second);
      REQUIRE(updates.size() == 2);
      REQUIRE(updates.back() == fixture.m_second);
      member.get_hub()->set(std::make_shared<PropertyHub>());
      REQUIRE(!attached->get());
      member.get_hub()->set(hub);
      REQUIRE(attached->get() == fixture.m_second);
      attachment_map.detach(hub);
      REQUIRE(!attached->get());
      REQUIRE(updates.size() == 5);
      REQUIRE(!updates.back());
    });
  }
}
