#include <queue>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Spire/Spire/AssociativeValueModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("AssociativeValueModel") {
  TEST_CASE("associate") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    auto bool_model2 = model.get_association("model2");
    bool_model1->set_current(true);
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model1");
    bool_model2->set_current(true);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model2");
    bool_model2->set_current(false);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    REQUIRE(model.get_current() == "");
  }

  TEST_CASE("reassociate_current") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    model.set_current("model1");
    REQUIRE(model.get_current() == "model1");
    auto bool_model2 = model.get_association("model1");
    REQUIRE(bool_model1 == bool_model2);
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model2->get_current());
    auto bool_model3 = model.get_association("model1");
    REQUIRE(bool_model2 == bool_model3);
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model3->get_current());
  }

  TEST_CASE("make_association") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    auto bool_model2 = model.get_association("model2");
    bool_model1->set_current(true);
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model1");
    bool_model2->set_current(true);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model2");
    bool_model2->set_current(false);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    REQUIRE(model.get_current() == "");
  }

  TEST_CASE("find") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    auto bool_model2 = model.get_association("model2");
    REQUIRE(model.find("model1") == bool_model1);
    REQUIRE(model.find("model2") == bool_model2);
    REQUIRE(model.find("model3") == nullptr);
  }

  TEST_CASE("get_state") {
    auto model = AssociativeValueModel<std::string>();
    REQUIRE(model.get_state() == QValidator::Acceptable);
  }

  TEST_CASE("get_current") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    REQUIRE(model.get_current() == "");
    bool_model1->set_current(true);
    REQUIRE(model.get_current() == "model1");
    bool_model1->set_current(false);
    REQUIRE(model.get_current() == "");
  }

  TEST_CASE("set_current") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    auto bool_model2 = model.get_association("model2");
    REQUIRE(model.get_current() == "");
    REQUIRE(model.set_current("model2") == QValidator::Acceptable);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(model.get_current() == "model2");
    REQUIRE(model.set_current("model1") == QValidator::Acceptable);
    REQUIRE(model.get_current() == "model1");
    REQUIRE_FALSE(bool_model2->get_current());
    REQUIRE(model.set_current("null") == QValidator::Invalid);
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
  }

  TEST_CASE("set_current_reentrant") {
    auto model = AssociativeValueModel<std::string>();
    auto updates = std::queue<std::string>();
    model.connect_current_signal([&] (auto value) {
      updates.push(value);
      if(value == "model1") {
        model.set_current("model2");
      }
    });
    model.get_association("model1");
    model.get_association("model2");
    model.set_current("model1");
    REQUIRE(updates.front() == "model1");
    updates.pop();
    REQUIRE(updates.front() == "model2");
    updates.pop();
    REQUIRE(updates.empty());
  }

  TEST_CASE("current_signal") {
    auto model = AssociativeValueModel<std::string>();
    auto current_value = std::string();
    auto signal_count = 0;
    model.connect_current_signal([&] (const auto& current) {
      ++signal_count;
      current_value = current;
    });
    auto bool_model1 = model.get_association("model1");
    auto bool_model2 = model.get_association("model2");
    model.set_current("model1");
    REQUIRE(current_value == "model1");
    REQUIRE(signal_count == 1);
    REQUIRE(current_value == "model1");
    REQUIRE(signal_count == 1);
    bool_model1->set_current(false);
    REQUIRE(current_value == "");
    REQUIRE(signal_count == 2);
    bool_model2->set_current(true);
    REQUIRE(current_value == "model2");
    REQUIRE(signal_count == 3);
    bool_model2->set_current(true);
    REQUIRE(current_value == "model2");
    REQUIRE(signal_count == 3);
  }

  TEST_CASE("default_value") {
    auto model = AssociativeValueModel<std::string>("default");
    auto default_model = model.get_association("default");
    auto bool_model = model.get_association("model1");
    bool_model->set_current(true);
    REQUIRE(model.get_current() == "model1");
    bool_model->set_current(false);
    REQUIRE(model.get_current() == "default");
    default_model->set_current(false);
    REQUIRE(model.get_current() == "default");
    REQUIRE(default_model->get_current());
  }

  TEST_CASE("default_instance_values") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.get_association("model1");
    REQUIRE(model.get_current() == "");
    REQUIRE_FALSE(bool_model1->get_current());
  }

  TEST_CASE("optional_model") {
    auto model = AssociativeValueModel<optional<std::string>>();
    auto bool_model1 = model.get_association(std::string("model1"));
    auto bool_model2 = model.get_association(std::string("model2"));
    REQUIRE_FALSE(model.get_current());
    bool_model1->set_current(true);
    REQUIRE(model.get_current() == std::string("model1"));
    REQUIRE_FALSE(bool_model2->get_current());
    bool_model1->set_current(false);
    REQUIRE_FALSE(model.get_current());
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
  }
}
