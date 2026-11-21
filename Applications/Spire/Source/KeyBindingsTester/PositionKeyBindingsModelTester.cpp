#include <vector>
#include <doctest/doctest.h>
#include "Spire/KeyBindings/PositionKeyBindingsModel.hpp"

using namespace Spire;

TEST_SUITE("PositionKeyBindingsModel") {
  TEST_CASE("defaults") {
    using Operation = PositionKeyBindingsModel::Operation;
    auto model = PositionKeyBindingsModel();
    REQUIRE(model.get_binding(Operation::FLATTEN)->get() ==
      QKeySequence(Qt::CTRL | Qt::Key_F));
    REQUIRE(model.get_binding(Operation::FLATTEN_ALL)->get() ==
      QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    REQUIRE(model.find_operation(Qt::CTRL | Qt::Key_F).value() ==
      Operation::FLATTEN);
    REQUIRE(model.find_operation(Qt::CTRL | Qt::SHIFT | Qt::Key_F).value() ==
      Operation::FLATTEN_ALL);
    REQUIRE(!model.find_operation(QKeySequence()));
    REQUIRE(!model.find_operation(Qt::Key_F));
  }

  TEST_CASE("binding_updates") {
    using Operation = PositionKeyBindingsModel::Operation;
    auto model = PositionKeyBindingsModel();
    auto flatten = model.get_binding(Operation::FLATTEN);
    auto all = model.get_binding(Operation::FLATTEN_ALL);
    auto updates = std::vector<QKeySequence>();
    flatten->connect_update_signal([&] (const auto& sequence) {
      updates.push_back(sequence);
    });
    REQUIRE(flatten->set(Qt::ALT | Qt::Key_F1) == QValidator::Acceptable);
    REQUIRE(!model.find_operation(Qt::CTRL | Qt::Key_F));
    REQUIRE(model.find_operation(Qt::ALT | Qt::Key_F1).value() ==
      Operation::FLATTEN);
    REQUIRE(all->set(Qt::ALT | Qt::Key_F1) == QValidator::Acceptable);
    REQUIRE(flatten->get().isEmpty());
    REQUIRE(model.find_operation(Qt::ALT | Qt::Key_F1).value() ==
      Operation::FLATTEN_ALL);
    REQUIRE(!model.find_operation(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    REQUIRE(updates == std::vector{
      QKeySequence(Qt::ALT | Qt::Key_F1), QKeySequence()});
    REQUIRE(all->set(Qt::ALT | Qt::Key_F1) == QValidator::Acceptable);
    REQUIRE(updates.size() == 2);
    REQUIRE(flatten->set(Qt::ALT | Qt::Key_F1) == QValidator::Acceptable);
    REQUIRE(all->get().isEmpty());
    REQUIRE(model.find_operation(Qt::ALT | Qt::Key_F1).value() ==
      Operation::FLATTEN);
    REQUIRE(flatten->set(QKeySequence()) == QValidator::Intermediate);
    REQUIRE(!model.find_operation(Qt::ALT | Qt::Key_F1));
    REQUIRE(!model.find_operation(QKeySequence()));
  }

  TEST_CASE("multi_key_sequence") {
    using Operation = PositionKeyBindingsModel::Operation;
    auto model = PositionKeyBindingsModel();
    auto binding = model.get_binding(Operation::FLATTEN);
    auto sequence = QKeySequence(Qt::CTRL | Qt::Key_F, Qt::Key_F);
    REQUIRE(binding->test(sequence) == QValidator::Invalid);
    REQUIRE(binding->set(sequence) == QValidator::Invalid);
    REQUIRE(binding->get() == QKeySequence(Qt::CTRL | Qt::Key_F));
    REQUIRE(model.find_operation(Qt::CTRL | Qt::Key_F).value() ==
      Operation::FLATTEN);
    REQUIRE(!model.find_operation(sequence));
  }

  TEST_CASE("retained_binding") {
    using Operation = PositionKeyBindingsModel::Operation;
    auto model = std::make_unique<PositionKeyBindingsModel>();
    auto flatten = model->get_binding(Operation::FLATTEN);
    auto all = model->get_binding(Operation::FLATTEN_ALL);
    model.reset();
    REQUIRE(flatten->set(Qt::CTRL | Qt::SHIFT | Qt::Key_F) ==
      QValidator::Acceptable);
    REQUIRE(all->get() == QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
  }
}
