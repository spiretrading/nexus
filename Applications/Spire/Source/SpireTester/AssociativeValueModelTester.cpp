#include <doctest/doctest.h>
#include <optional>
#include <boost/optional/optional_io.hpp>
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
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model1");
    bool_model2->set_current(true);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model2");
    bool_model2->set_current(false);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model2");
    bool_model1->set_current(false);
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(bool_model2->get_current());
    REQUIRE(model.get_current() == "model2");
  }

  TEST_CASE("make_association") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = model.make_association("model1");
    auto bool_model2 = model.make_association("model2");
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
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
    REQUIRE(bool_model2->get_current());
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
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    model.associate(bool_model2, "model2");
    REQUIRE(model.get_current() == "model1");
    bool_model2->set_current(true);
    REQUIRE(model.get_current() == "model2");
  }

  TEST_CASE("set_current") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    model.associate(bool_model2, "model2");
    REQUIRE(model.get_current() == "model1");
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

  TEST_CASE("current_signal") {
    auto model = AssociativeValueModel<std::string>();
    auto current_value = std::string();
    auto signal_count = 0;
    model.connect_current_signal([&] (const auto& current) {
      ++signal_count;
      current_value = current;
    });
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    REQUIRE(current_value == "model1");
    REQUIRE(signal_count == 1);
    model.associate(bool_model2, "model2");
    REQUIRE(current_value == "model1");
    REQUIRE(signal_count == 1);
    bool_model1->set_current(false);
    REQUIRE(current_value == "model1");
    REQUIRE(signal_count == 1);
    bool_model2->set_current(true);
    REQUIRE(current_value == "model2");
    REQUIRE(signal_count == 2);
    bool_model2->set_current(true);
    REQUIRE(current_value == "model2");
    REQUIRE(signal_count == 2);
  }

  TEST_CASE("default_instance_values") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model1->get_current());
    auto bool_model2 = std::make_shared<LocalBooleanModel>(true);
    model.associate(bool_model2, "model2");
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
  }

  TEST_CASE("default_value") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    model.associate(bool_model2, "model2");
    model.set_default_value("model2");
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    bool_model1->set_current(false);
    REQUIRE(model.get_current() == "model2");
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE(bool_model2->get_current());
  }

  TEST_CASE("reentrant") {
    auto model = AssociativeValueModel<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(bool_model1, "model1");
    model.associate(bool_model2, "model2");
    model.connect_current_signal([=] (const auto& current) {
      if(current == "model1") {
        bool_model2->set_current(true);
      } else {
        bool_model1->set_current(true);
      }
    });
    bool_model2->set_current(true);
    REQUIRE(model.get_current() == "model2");
    bool_model2->set_current(false);
    REQUIRE(model.get_current() == "model2");
    bool_model1->set_current(true);
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
  }

  TEST_CASE("default_reentrant") {
    auto model = AssociativeValueModel<std::string>();
    auto null_model = std::make_shared<LocalBooleanModel>(false);
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model.associate(null_model, "null");
    model.associate(bool_model1, "model1");
    model.associate(bool_model2, "model2");
    model.set_default_value("null");
    model.connect_current_signal([=] (const auto& current) {
      if(current == std::string("model1")) {
        bool_model2->set_current(true);
      } else {
        bool_model1->set_current(true);
      }
    });
    bool_model2->set_current(true);
    REQUIRE(model.get_current() == "model2");
    bool_model1->set_current(true);
    REQUIRE(model.get_current() == "model1");
    REQUIRE(bool_model1->get_current());
    REQUIRE_FALSE(null_model->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
    bool_model1->set_current(false);
    REQUIRE(model.get_current() == "null");
    REQUIRE(null_model->get_current());
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
  }

  TEST_CASE("nullable_model") {
    auto model = make_nullable_associative_model<std::string>();
    auto bool_model1 = std::make_shared<LocalBooleanModel>(false);
    auto bool_model2 = std::make_shared<LocalBooleanModel>(false);
    model->associate(bool_model1, std::string("model1"));
    model->associate(bool_model2, std::string("model2"));
    REQUIRE_FALSE(model->get_current());
    bool_model1->set_current(true);
    REQUIRE(model->get_current() == std::string("model1"));
    REQUIRE_FALSE(bool_model2->get_current());
    bool_model1->set_current(false);
    REQUIRE_FALSE(model->get_current());
    REQUIRE_FALSE(bool_model1->get_current());
    REQUIRE_FALSE(bool_model2->get_current());
  }
}
