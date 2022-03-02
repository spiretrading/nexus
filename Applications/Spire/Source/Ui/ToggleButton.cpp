#include "Spire/Ui/ToggleButton.hpp"

using namespace Spire;

ToggleButton::ToggleButton(QWidget* body, QString tooltip, QWidget* parent) {}

ToggleButton::ToggleButton(QWidget* body, QWidget* parent) {}

ToggleButton::ToggleButton(QWidget* body, std::shared_ptr<BooleanModel> current,
  QString tooltip, QWidget* parent) {}

const QWidget& ToggleButton::get_body() const {
  return *m_body;
}

QWidget& ToggleButton::get_body() {
  return *m_body;
}

const std::shared_ptr<BooleanModel>& ToggleButton::get_current() const {
  return m_current;
}

ToggleButton* Spire::make_icon_toggle_button(QImage icon, QWidget* parent) {
  return nullptr;
}

ToggleButton* Spire::make_icon_toggle_button(
    QImage icon, QString tooltip, QWidget* parent) {
  return nullptr;
}
