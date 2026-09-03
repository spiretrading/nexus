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
      auto top = PropertyHubMember(
        roster, widget, "", std::make_shared<PropertyHub>());
      {
        auto middle = PropertyHubMember(
          roster, widget, "", std::make_shared<PropertyHub>());
        auto bottom = PropertyHubMember(
          roster, widget, "", std::make_shared<PropertyHub>());
        REQUIRE(roster->get_size() == 3);
        REQUIRE(roster->get(1) == &middle);
      }
      REQUIRE(roster->get_size() == 1);
      REQUIRE(roster->get(0) == &top);
    });
  }

  TEST_CASE("component") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      widget.setWindowTitle("Book View");
      auto member = PropertyHubMember(roster, widget,
        ":/Icons/bookview.svg", std::make_shared<PropertyHub>());
      REQUIRE(&member.get_component() == &widget);
      REQUIRE(member.get_name() == "Book View");
      REQUIRE(member.get_icon_path() == ":/Icons/bookview.svg");
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
      hub->get<int>("count")->set(7);
      REQUIRE(count->get() == 7);
    });
  }

  TEST_CASE("share") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      auto hub = std::make_shared<PropertyHub>();
      auto left = PropertyHubMember(roster, widget, "", hub);
      auto right = PropertyHubMember(roster, widget, "", hub);
      auto left_count = left.get_property<int>("count");
      auto right_count = right.get_property<int>("count");
      left_count->set(5);
      REQUIRE(right_count->get() == 5);
      right_count->set(9);
      REQUIRE(left_count->get() == 9);
    });
  }

  TEST_CASE("join") {
    run_test([] {
      auto roster = std::make_shared<ArrayListModel<PropertyHubMember*>>();
      auto widget = QWidget();
      auto origin = std::make_shared<PropertyHub>();
      auto member = PropertyHubMember(roster, widget, "", origin);
      auto count = member.get_property<int>("count");
      count->set(5);
      auto populated = std::make_shared<PropertyHub>();
      populated->get<int>("count")->set(9);
      member.get_hub()->set(populated);
      REQUIRE(count->get() == 9);
      populated->get<int>("count")->set(13);
      REQUIRE(count->get() == 13);
      origin->get<int>("count")->set(21);
      REQUIRE(count->get() == 13);
      auto empty = std::make_shared<PropertyHub>();
      member.get_hub()->set(empty);
      REQUIRE(count->get() == 13);
      REQUIRE(empty->get<int>("count")->get() == 13);
    });
  }
}
