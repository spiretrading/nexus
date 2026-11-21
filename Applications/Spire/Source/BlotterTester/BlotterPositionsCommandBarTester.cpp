#include <QtTest/qtestmouse.h>
#include "Spire/Blotter/BlotterPositionsCommandBar.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;

namespace {
  using FlattenState = BlotterPositionsCommandBar::FlattenState;

  struct Fixture {
    std::shared_ptr<ValueModel<FlattenState>> m_can_flatten;
    std::shared_ptr<PositionKeyBindingsModel> m_bindings;
    std::unique_ptr<BlotterPositionsCommandBar> m_bar;

    Fixture()
      : Fixture(FlattenState::ALL) {}

    explicit Fixture(FlattenState state)
        : m_can_flatten(std::make_shared<LocalValueModel<FlattenState>>(state)),
          m_bindings(std::make_shared<PositionKeyBindingsModel>()) {
      initialize_resources();
      m_bar = std::make_unique<BlotterPositionsCommandBar>(
        m_can_flatten, m_bindings);
      m_bar->resize(300, m_bar->sizeHint().height());
      m_bar->show();
      QApplication::processEvents();
    }

    Button& find_button(const QString& name) {
      for(auto widget : m_bar->findChildren<QWidget*>()) {
        if(widget->accessibleName() == name) {
          return dynamic_cast<Button&>(*widget);
        }
      }
      throw std::runtime_error("Button not found.");
    }

    void click(Button& button) {
      auto point = button.mapToGlobal(button.rect().center());
      QCursor::setPos(point);
      QTest::mouseClick(m_bar->windowHandle(), Qt::LeftButton,
        Qt::NoModifier, m_bar->mapFromGlobal(point));
      QApplication::processEvents();
    }
  };

  TextBox& find_tooltip(const QString& text) {
    for(auto window : QApplication::topLevelWidgets()) {
      for(auto widget : window->findChildren<QWidget*>()) {
        if(auto label = dynamic_cast<TextBox*>(widget)) {
          if(label->get_current()->get().startsWith(text)) {
            return *label;
          }
        }
      }
    }
    throw std::runtime_error("Tooltip not found.");
  }
}

TEST_SUITE("BlotterPositionsCommandBar") {
  UI_TEST_CASE("initial_flatten_state") {
    for(auto state : {FlattenState::NONE, FlattenState::ALL,
        FlattenState::SELECTED_AND_ALL}) {
      auto fixture = Fixture(state);
      auto& flatten = fixture.find_button("Flatten position");
      auto& all = fixture.find_button("Flatten all positions");
      REQUIRE(flatten.isEnabled() == (state == FlattenState::SELECTED_AND_ALL));
      REQUIRE(all.isEnabled() == (state != FlattenState::NONE));
    }
  }

  UI_TEST_CASE("flatten_commands") {
    auto fixture = Fixture();
    auto& flatten = fixture.find_button("Flatten position");
    auto& all = fixture.find_button("Flatten all positions");
    auto flatten_count = 0;
    auto all_count = 0;
    fixture.m_bar->connect_flatten_signal([&] { ++flatten_count; });
    fixture.m_bar->connect_flatten_all_signal([&] { ++all_count; });
    REQUIRE_FALSE(flatten.isEnabled());
    REQUIRE(all.isEnabled());
    fixture.click(flatten);
    REQUIRE(flatten_count == 0);
    fixture.click(all);
    REQUIRE(all_count == 1);
    REQUIRE(flatten_count == 0);
    fixture.m_can_flatten->set(FlattenState::SELECTED_AND_ALL);
    REQUIRE(flatten.isEnabled());
    REQUIRE(all.isEnabled());
    fixture.click(flatten);
    REQUIRE(flatten_count == 1);
    REQUIRE(all_count == 1);
    fixture.m_can_flatten->set(FlattenState::NONE);
    REQUIRE_FALSE(flatten.isEnabled());
    REQUIRE_FALSE(all.isEnabled());
    fixture.click(flatten);
    fixture.click(all);
    REQUIRE(flatten_count == 1);
    REQUIRE(all_count == 1);
    fixture.m_can_flatten->set(FlattenState::ALL);
    REQUIRE_FALSE(flatten.isEnabled());
    REQUIRE(all.isEnabled());
    fixture.click(all);
    REQUIRE(all_count == 2);
  }

  UI_TEST_CASE("shortcut_tooltips") {
    using Operation = PositionKeyBindingsModel::Operation;
    auto fixture = Fixture();
    auto& all = fixture.find_button("Flatten all positions");
    auto& flatten_tooltip = find_tooltip("Flatten position");
    auto& all_tooltip = find_tooltip("Flatten all positions");
    auto sequence = QKeySequence(Qt::CTRL | Qt::Key_F);
    REQUIRE(flatten_tooltip.get_current()->get() ==
      QString("Flatten position (%1)").arg(
        sequence.toString(QKeySequence::NativeText)));
    REQUIRE(all_tooltip.get_current()->get() ==
      QString("Flatten all positions (%1)").arg(
        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F).
          toString(QKeySequence::NativeText)));
    fixture.m_bindings->get_binding(Operation::FLATTEN_ALL)->set(sequence);
    REQUIRE(flatten_tooltip.get_current()->get() == "Flatten position");
    REQUIRE(all_tooltip.get_current()->get() ==
      QString("Flatten all positions (%1)").arg(
        sequence.toString(QKeySequence::NativeText)));
    fixture.m_bindings->get_binding(Operation::FLATTEN_ALL)->set(
      QKeySequence());
    REQUIRE(all_tooltip.get_current()->get() == "Flatten all positions");
    REQUIRE(all.isEnabled());
    auto count = 0;
    fixture.m_bar->connect_flatten_all_signal([&] { ++count; });
    fixture.click(all);
    REQUIRE(count == 1);
  }
}
