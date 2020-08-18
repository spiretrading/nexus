#include "Spire/Ui/NumericInputWidget.hpp"
#include <QKeyEvent>
#include <QLineEdit>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  const auto DEFAULT_DECIMAL_PLACES = 6;
  const auto SHIFT_STEPS = 10;
  const auto DOWN_ARROW_ICON = ":/Icons/arrow-down.svg";
  const auto DOWN_ARROW_HOVER_ICON = ":/Icons/arrow-down-hover.svg";
  const auto DOWN_ARROW_DISABLED_ICON = ":/Icons/arrow-down-disabled.svg";
  const auto UP_ARROW_ICON = ":/Icons/arrow-up.svg";
  const auto UP_ARROW_HOVER_ICON = ":/Icons/arrow-up-hover.svg";
  const auto UP_ARROW_DISABLED_ICON = ":/Icons/arrow-up-disabled.svg";

  auto ARROW_WIDTH() {
    static auto width = scale_width(8);
    return width;
  }

  auto clamped_real(NumericInputWidget::Real value,
      NumericInputWidget::Real min, NumericInputWidget::Real max) {
    if(value.compare(min) < 0) {
      return min;
    } else if(value.compare(max) > 0) {
      return max;
    }
    return value;
  }
}

NumericInputWidget::NumericInputWidget(Real value, QWidget* parent)
    : QAbstractSpinBox(parent),
      m_minimum(std::numeric_limits<Real>::lowest()),
      m_maximum(std::numeric_limits<Real>::max()),
      m_step(1) {
  lineEdit()->setText(display_string(value));
  set_decimals(DEFAULT_DECIMAL_PLACES);
  update_stylesheet();
  connect(this, &QAbstractSpinBox::valueChanged, [=] (auto value) {});
}

void NumericInputWidget::focusOutEvent(QFocusEvent* event) {
  on_editing_finished();
}

void NumericInputWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Backspace:
      lineEdit()->backspace();
      return;
    case Qt::Key_Delete:
      return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      on_editing_finished();
      return;
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
      return;
    case Qt::Key_Up:
      add_step(1, event->modifiers());
      return;
    case Qt::Key_Down:
      add_step(-1, event->modifiers());
      return;
    case Qt::Key_Left:
    case Qt::Key_Right:
      QAbstractSpinBox::keyPressEvent(event);
      return;
  }
  if(!event->text().isEmpty()) {
    auto input = event->text().at(0);
    if(input.isNumber() || input == m_locale.negativeSign() ||
        input == m_locale.decimalPoint()) {
      auto current_text = lineEdit()->text().remove(
        lineEdit()->selectedText());
      current_text.insert(lineEdit()->cursorPosition(), input);
      if(is_valid(current_text)) {
        QAbstractSpinBox::keyPressEvent(event);
      }
    }
  }
}

QAbstractSpinBox::StepEnabled NumericInputWidget::stepEnabled() const {
  return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
}

void NumericInputWidget::set_decimals(int decimals) {
  m_decimals = std::min(decimals, MAX_DECIMAL_PLACES);
  auto point_count = [&] {
    if(m_decimals > 0) {
      return 1;
    }
    return 0;
  }();
  m_real_regex = QRegularExpression(QString(
    "^[-]?([0-9]*[.]{0,%1}[0-9]{0,%2})$").arg(point_count).arg(m_decimals));
}

void NumericInputWidget::set_minimum_decimals(int decimals) {
  m_minimum_decimals = decimals;
}

void NumericInputWidget::set_minimum(Real minimum) {
  m_minimum = minimum;
}

void NumericInputWidget::set_maximum(Real maximum) {
  m_maximum = maximum;
}

void NumericInputWidget::set_step(Real step) {
  m_step = step;
}

void NumericInputWidget::stepBy(int step) {
  add_step(step);
}

void NumericInputWidget::add_step(int step) {
  add_step(step, Qt::NoModifier);
}

void NumericInputWidget::add_step(int step, Qt::KeyboardModifiers modifiers) {
  step = [&] {
    if(modifiers == Qt::ShiftModifier) {
      return step * SHIFT_STEPS;
    }
    return step;
  }();
  if(text().isEmpty()) {
    auto value = Real(step);
    value *= m_step;
    lineEdit()->setText(display_string(value));
    lineEdit()->setCursorPosition(text().length());
    return;
  }
  auto value = Real(lineEdit()->text().toStdString().c_str());
  value += step;
  value = clamped_real(value, m_minimum, m_maximum);
  lineEdit()->setText(display_string(value));
  lineEdit()->setCursorPosition(text().length());
}

QString NumericInputWidget::display_string(Real value) {
  auto str = text();
  str.remove(QRegExp("0+$"));
  str.remove(QRegExp("\\.$"));
  if(str.contains(m_locale.decimalPoint())) {
    return QString::fromStdString(value.str(
      str.length() - str.indexOf(m_locale.decimalPoint()) - 1,
      std::ios_base::fixed));
  }
  return QString::fromStdString(value.str(text().length() + 1,
    std::ios_base::dec));
}

bool NumericInputWidget::is_valid(const QString& text) {
  // TODO: create localized version of this string?
  if(!text.contains(m_real_regex)) {
    return false;
  }
  if(!text.contains(QRegularExpression("[0-9]"))) {
    return true;
  }
  auto value = Real(text.toStdString().c_str());
  return value.compare(m_minimum) >= 0 && value.compare(m_maximum) <= 0;
}

void NumericInputWidget::set_stylesheet(bool is_up_disabled,
    bool is_down_disabled) {
  auto [up_icon, up_icon_hover] = [&] () -> std::pair<QString, QString> {
    if(is_up_disabled) {
      return {UP_ARROW_DISABLED_ICON, UP_ARROW_DISABLED_ICON};
    }
    return {UP_ARROW_ICON, UP_ARROW_HOVER_ICON};
  }();
  auto [down_icon, down_icon_hover] = [&] () -> std::pair<QString, QString> {
    if(is_down_disabled) {
      return {DOWN_ARROW_DISABLED_ICON, DOWN_ARROW_DISABLED_ICON};
    }
    return {DOWN_ARROW_ICON, DOWN_ARROW_HOVER_ICON};
  }();
  setStyleSheet(QString(R"(
    QAbstractSpinBox {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %5px;
      padding-left: %4px;
    }

    QAbstractSpinBox:focus {
      border: %1px solid #4B23A0;
    }

    QAbstractSpinBox::up-button {
      border: none;
    }

    QAbstractSpinBox::down-button {
      border: none;
    }

    QAbstractSpinBox::up-arrow {
      height: %2px;
      image: url(%7);
      padding-top: %6px;
      width: %3px;
    }

    QAbstractSpinBox::up-arrow:disabled {
      height: %2px;
      image: url(%11);
      padding-top: %6px;
      width: %3px;
    }

    QAbstractSpinBox::up-arrow:hover {
      height: %2px;
      image: url(%8);
      padding-top: %6px;
      width: %3px;
    }

    QAbstractSpinBox::down-arrow {
      height: %2px;
      image: url(%9);
      width: %3px;
    }

    QAbstractSpinBox::down-arrow:disabled {
      height: %2px;
      image: url(%12);
      width: %3px;
    }

    QAbstractSpinBox::down-arrow:hover {
      height: %2px;
      image: url(%10);
      width: %3px;
    })").arg(scale_width(1)).arg(scale_height(8)).arg(ARROW_WIDTH())
        .arg(scale_width(10)).arg(scale_height(12)).arg(scale_height(4))
        .arg(up_icon).arg(up_icon_hover).arg(down_icon).arg(down_icon_hover)
        .arg(UP_ARROW_DISABLED_ICON).arg(DOWN_ARROW_DISABLED_ICON));
}

void NumericInputWidget::update_stylesheet() {
  auto value = Real(lineEdit()->text().toStdString().c_str());
  if(value.compare(m_minimum) == 0) {
    set_stylesheet(false, true);
  } else if(value.compare(m_maximum) == 0) {
    set_stylesheet(true, false);
  } else {
    set_stylesheet(false, false);
  }
}

void NumericInputWidget::on_editing_finished() {
  if(text().isEmpty()) {
    lineEdit()->setText("0");
  }
  auto value = Real(text().toStdString().c_str());
  lineEdit()->setText(display_string(value));
  Q_EMIT editingFinished();
}
