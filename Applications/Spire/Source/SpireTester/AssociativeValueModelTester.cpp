#include <doctest/doctest.h>
#include "Spire/Ui/AssociativeValueModel.hpp"
#include "Spire/Ui/LocalValueModel.hpp"

using namespace Spire;

TEST_SUITE("AssociativeValueModel") {
  TEST_CASE("associate") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    model.associate(bool_model2, "model2");
    bool_model1->set_current(true);
    REQUIRE(bool_model1->get_current() == true);
    REQUIRE(bool_model2->get_current() == false);
    REQUIRE(model.get_current() == "model1");
    bool_model2->set_current(true);
    REQUIRE(bool_model1->get_current() == false);
    REQUIRE(bool_model2->get_current() == true);
    REQUIRE(model.get_current() == "model2");
    bool_model2->set_current(false);
    REQUIRE(bool_model1->get_current() == false);
    REQUIRE(bool_model2->get_current() == true);
    REQUIRE(model.get_current() == "model2");
  }

  TEST_CASE("make_association") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.make_association("model1");
    auto bool_model2 = model.make_association("model2");
    REQUIRE(bool_model1->get_current() == true);
    REQUIRE(bool_model2->get_current() == false);
    bool_model1->set_current(true);
    REQUIRE(bool_model1->get_current() == true);
    REQUIRE(bool_model2->get_current() == false);
    REQUIRE(model.get_current() == "model1");
    bool_model2->set_current(true);
    REQUIRE(bool_model1->get_current() == false);
    REQUIRE(bool_model2->get_current() == true);
    REQUIRE(model.get_current() == "model2");
    bool_model2->set_current(false);
    REQUIRE(bool_model1->get_current() == false);
    REQUIRE(bool_model2->get_current() == true);
    REQUIRE(model.get_current() == "model2");
  }

  TEST_CASE("find") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.make_association("model1");
    auto bool_model2 = model.make_association("model2");
    REQUIRE(model.find("model1") == bool_model1);
    REQUIRE(model.find("model2") == bool_model2);
    REQUIRE(model.find("model3") == nullptr);
  }

  TEST_CASE("get_state") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model = std::make_shared<LocalBooleanModel>(false);
    REQUIRE(model.get_state() == QValidator::Invalid);
    model.associate(bool_model, "bool_model");
    REQUIRE(model.get_state() == QValidator::Acceptable);
  }

  TEST_CASE("get_current") {
  
  }

  TEST_CASE("set_current") {
  
  }

  TEST_CASE("current_signal") {
  
  }

  TEST_CASE("reentrant") {
  
  }
}
