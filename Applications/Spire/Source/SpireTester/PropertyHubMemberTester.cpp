#include <any>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/PropertyHubMember.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_SUITE("PropertyHubMember") {
  TEST_CASE("roster") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      {
        auto member = PropertyHubMember(
          roster, widget, "", std::make_shared<PropertyHub>());
        REQUIRE(roster->get_size() == 1);
        REQUIRE(roster->get(0) == &member);
      }
      REQUIRE(roster->get_size() == 0);
    });
  }

  TEST_CASE("name") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      widget.setWindowTitle("Book View");
      auto member = PropertyHubMember(
        roster, widget, "", std::make_shared<PropertyHub>());
      REQUIRE(member.get_name() == "Book View");
    });
  }

  TEST_CASE("get_property") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      auto hub = std::make_shared<PropertyHub>();
      auto member = PropertyHubMember(roster, widget, "", hub);
      auto count = member.get_property<int>("count");
      REQUIRE(count->get() == 0);
      REQUIRE(member.get_property<int>("count") == count);
      REQUIRE_THROWS_AS(
        member.get_property<double>("count"), std::bad_any_cast);
      count->set(5);
      REQUIRE(hub->get<int>("count")->get() == 5);
    });
  }

  TEST_CASE("join") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      auto member = PropertyHubMember(
        roster, widget, "", std::make_shared<PropertyHub>());
      auto count = member.get_property<int>("count");
      count->set(5);
      auto hub = std::make_shared<PropertyHub>();
      hub->get<int>("count")->set(9);
      member.get_hub()->set(hub);
      REQUIRE(count->get() == 9);
      count->set(11);
      REQUIRE(hub->get<int>("count")->get() == 11);
    });
  }

  TEST_CASE("unlink") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      auto member = PropertyHubMember(
        roster, widget, "", std::make_shared<PropertyHub>());
      auto count = member.get_property<int>("count");
      count->set(5);
      auto hub = std::make_shared<PropertyHub>();
      member.get_hub()->set(hub);
      REQUIRE(count->get() == 5);
      REQUIRE(hub->get<int>("count")->get() == 5);
    });
  }
}
