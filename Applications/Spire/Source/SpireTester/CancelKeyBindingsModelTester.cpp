#include <doctest/doctest.h>
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("CancelKeyBindingsModel") {
  TEST_CASE("access") {
    auto model = CancelKeyBindingsModel();
    auto most_recent_binding =
      model.get_binding(CancelKeyBindingsModel::Operation::MOST_RECENT);
    REQUIRE(most_recent_binding->get().isEmpty());
    most_recent_binding->set(Qt::Key_F1);
    REQUIRE(most_recent_binding->get() == QKeySequence(Qt::Key_F1));
    most_recent_binding->set(Qt::Key_Escape);
    REQUIRE(most_recent_binding->get() == QKeySequence(Qt::Key_Escape));
    auto all_binding =
      model.get_binding(CancelKeyBindingsModel::Operation::ALL);
    REQUIRE(all_binding->get().isEmpty());
    all_binding->set(Qt::CTRL | Qt::Key_F10);
    REQUIRE(all_binding->get() == QKeySequence(Qt::CTRL | Qt::Key_F10));
    REQUIRE(model.find_operation(Qt::Key_Escape).get() ==
      CancelKeyBindingsModel::Operation::MOST_RECENT);
    REQUIRE(model.find_operation(Qt::CTRL | Qt::Key_F10).get() ==
      CancelKeyBindingsModel::Operation::ALL);
    REQUIRE(!model.find_operation(Qt::Key_F10));
    REQUIRE(!model.find_operation(Qt::Key_0));
  }

  TEST_CASE("validate1") {
    auto model = CancelKeyBindingsModel();
    auto binding = model.get_binding(CancelKeyBindingsModel::Operation::ALL);
    REQUIRE(binding->set(Qt::Key_F1) == QValidator::Acceptable);
    REQUIRE(binding->get() == QKeySequence(Qt::Key_F1));
    REQUIRE(binding->set(Qt::CTRL | Qt::ALT | Qt::Key_F1) ==
      QValidator::Acceptable);
    REQUIRE(binding->get() == QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F1));
    REQUIRE(binding->set(Qt::ALT | Qt::Key_F12) == QValidator::Acceptable);
    REQUIRE(binding->get() == QKeySequence(Qt::ALT | Qt::Key_F12));
    REQUIRE(binding->set(Qt::SHIFT | Qt::Key_5) == QValidator::Acceptable);
    REQUIRE(binding->get() == QKeySequence(Qt::SHIFT | Qt::Key_5));
    REQUIRE(binding->set(Qt::CTRL | Qt::SHIFT | Qt::Key_2) ==
      QValidator::Acceptable);
    REQUIRE(binding->get() == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_2));
    REQUIRE(binding->set(Qt::Key_Escape) == QValidator::Acceptable);
    REQUIRE(binding->get() == QKeySequence(Qt::Key_Escape));
    REQUIRE(binding->set(Qt::Key_A) == QValidator::Invalid);
    REQUIRE(binding->set(Qt::ALT | Qt::Key_A) == QValidator::Invalid);
    REQUIRE(binding->set(Qt::CTRL | Qt::Key_A) == QValidator::Invalid);
    REQUIRE(binding->set(Qt::SHIFT | Qt::Key_A) == QValidator::Invalid);
    REQUIRE(binding->set(Qt::Key_0) == QValidator::Invalid);
    REQUIRE(binding->set(Qt::Key_Tab) == QValidator::Invalid);
    REQUIRE(binding->set(Qt::ALT | Qt::Key_Escape) == QValidator::Invalid);
  }

  TEST_CASE("validate2") {
    auto model = CancelKeyBindingsModel();
    auto most_recent_binding =
      model.get_binding(CancelKeyBindingsModel::Operation::MOST_RECENT);
    most_recent_binding->set(Qt::Key_F1);
    REQUIRE(most_recent_binding->get() == Qt::Key_F1);
    REQUIRE(model.find_operation(Qt::Key_F1).get() ==
      CancelKeyBindingsModel::Operation::MOST_RECENT);
    auto all_binding =
      model.get_binding(CancelKeyBindingsModel::Operation::ALL);
    all_binding->set(Qt::Key_F1);
    REQUIRE(most_recent_binding->get().isEmpty());
    REQUIRE(all_binding->get() == Qt::Key_F1);
    REQUIRE(model.find_operation(Qt::Key_F1).get() ==
      CancelKeyBindingsModel::Operation::ALL);
    all_binding->set(Qt::Key_F1);
    REQUIRE(most_recent_binding->get().isEmpty());
    REQUIRE(all_binding->get() == Qt::Key_F1);
    REQUIRE(model.find_operation(Qt::Key_F1).get() ==
      CancelKeyBindingsModel::Operation::ALL);
  }
}
