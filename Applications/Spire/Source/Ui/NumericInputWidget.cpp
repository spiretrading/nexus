#include "Spire/Ui/NumericInputWidget.hpp"
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QLineEdit>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
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
      m_minimum(std::numeric_limits<long double>::lowest()),
      m_maximum(std::numeric_limits<long double>::max()),
      m_step(1),
      m_last_valid_value(value),
      m_has_first_click(false) {
  connect(lineEdit(), &QLineEdit::textChanged, this,
    &NumericInputWidget::on_text_changed);
  setContextMenuPolicy(Qt::NoContextMenu);
  lineEdit()->setText(display_string(m_last_valid_value));
  set_decimals(DEFAULT_DECIMAL_PLACES);
  lineEdit()->installEventFilter(this);
}

void NumericInputWidget::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_stylesheet();
  }
}

bool NumericInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == lineEdit() && event->type() == QEvent::MouseButtonPress &&
      !m_has_first_click && isEnabled()) {
    auto e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton) {
      m_has_first_click = true;
      selectAll();
      return true;
    }
  }
  return QAbstractSpinBox::eventFilter(watched, event);
}

void NumericInputWidget::focusInEvent(QFocusEvent* event) {
  if(event->reason() != Qt::MouseFocusReason) {
    m_has_first_click = true;
  }
  QAbstractSpinBox::focusInEvent(event);
}

void NumericInputWidget::focusOutEvent(QFocusEvent* event) {
  m_has_first_click = false;
  blockSignals(true);
  QAbstractSpinBox::focusOutEvent(event);
  blockSignals(false);
  on_editing_finished();
}

void NumericInputWidget::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers().testFlag(Qt::ControlModifier)) {
    QAbstractSpinBox::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
    case Qt::Key_Backspace:
      lineEdit()->backspace();
      return;
    case Qt::Key_Delete:
      lineEdit()->setText(display_string(m_last_valid_value));
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
      if(is_valid(current_text.trimmed())) {
        QAbstractSpinBox::keyPressEvent(event);
      }
    }
  }
}

void NumericInputWidget::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && !m_has_first_click) {
    selectAll();
    m_has_first_click = true;
  }
  QAbstractSpinBox::mousePressEvent(event);
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
    "^[%1]?([0-9]*[%2]{0,%3}[0-9]{0,%4})$").arg(m_locale.negativeSign())
    .arg(m_locale.decimalPoint()).arg(point_count).arg(m_decimals));
}

void NumericInputWidget::set_minimum_decimals(int decimals) {
  m_minimum_decimals = decimals;
}

void NumericInputWidget::set_minimum(Real minimum) {
  m_minimum = minimum;
  auto value = get_value(text());
  if(value != boost::none && value->compare(m_minimum) < 0) {
    m_last_valid_value = m_minimum;
    lineEdit()->setText(display_string(m_minimum));
  }
}

void NumericInputWidget::set_maximum(Real maximum) {
  m_maximum = maximum;
  auto value = get_value(text());
  if(value != boost::none && value->compare(m_maximum) > 0) {
    m_last_valid_value = m_maximum;
    lineEdit()->setText(display_string(m_maximum));
  }
}

void NumericInputWidget::set_step(Real step) {
  m_step = step;
}


NumericInputWidget::Real NumericInputWidget::get_value() const {
  return m_last_valid_value;
}

void NumericInputWidget::set_value(Real value) {
  blockSignals(true);
  value = clamped_real(value, m_minimum, m_maximum);
  lineEdit()->setText(display_string(value));
  blockSignals(false);
}

void NumericInputWidget::stepBy(int step) {
  add_step(step);
}

connection NumericInputWidget::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
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
  auto value = get_value(text());
  if(text().isEmpty() || value == boost::none) {
    auto value = Real(step);
    value *= m_step;
    lineEdit()->setText(display_string(value));
    lineEdit()->setCursorPosition(text().length());
    return;
  }
  *value += step;
  value = clamped_real(*value, m_minimum, m_maximum);
  lineEdit()->setText(display_string(*value));
  lineEdit()->setCursorPosition(text().length());
}

QString NumericInputWidget::display_string(Real value) {
  auto str = text();
  str.remove(QRegularExpression("0+$"));
  str.remove(QRegularExpression(
    QString("\\%1$").arg(m_locale.decimalPoint())));
  if(str.contains(m_locale.decimalPoint())) {
    auto displayed_text = QString::fromStdString(value.str(
      str.length() - str.indexOf(m_locale.decimalPoint()) - 1,
      std::ios_base::fixed));
    return displayed_text.remove(
      QRegularExpression(QString("\\%1%2*$").arg(m_locale.decimalPoint())
      .arg(0))).trimmed();
  }
  return QString::fromStdString(value.str(text().length() + 1,
    std::ios_base::dec));
}

boost::optional<NumericInputWidget::Real>
    NumericInputWidget::get_value(const QString& text) const {
  try {
    return Real(text.toStdString().c_str());
  } catch (const std::runtime_error&) {
    return boost::none;
  }
}

bool NumericInputWidget::is_valid(const QString& text) {
  if(!text.contains(m_real_regex)) {
    return false;
  }
  if(!text.contains(QRegularExpression("[0-9]"))) {
    return true;
  }
  auto value = get_value(text);
  if(value == boost::none) {
    return true;
  }
  return value->compare(m_minimum) >= 0 && value->compare(m_maximum) <= 0;
}

void NumericInputWidget::set_stylesheet(bool is_up_disabled,
    bool is_down_disabled) {
  if(isEnabled()) {
    setButtonSymbols(UpDownArrows);
  } else {
    setButtonSymbols(NoButtons);
  }
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
  auto value = get_value(text());
  if(value == boost::none) {
    set_stylesheet(false, false);
  } else if(value->compare(m_minimum) == 0) {
    set_stylesheet(false, true);
  } else if(value->compare(m_maximum) == 0) {
    set_stylesheet(true, false);
  } else {
    set_stylesheet(false, false);
  }
}

void NumericInputWidget::on_editing_finished() {
  auto value = get_value(text());
  if(text().isEmpty() || value == boost::none) {
    lineEdit()->setText(display_string(m_last_valid_value));
  } else {
    m_last_valid_value = *value;
    lineEdit()->setText(display_string(m_last_valid_value));
  }
  Q_EMIT editingFinished();
}

void NumericInputWidget::on_text_changed(const QString& text) {
  if(!text.contains(m_real_regex)) {
    lineEdit()->blockSignals(true);
    lineEdit()->setText(m_last_valid_text);
    lineEdit()->blockSignals(false);
    return;
  }
  m_last_valid_text = text;
  update_stylesheet();
  auto value = get_value(m_last_valid_text);
  if(value != boost::none) {
    m_change_signal(*value);
  }
}

void NumericInputWidget::on_text_pasted() {
  auto text = qApp->clipboard()->text();
  if(!text.isEmpty()) {
    auto potential_text = lineEdit()->text().remove(
      lineEdit()->selectedText()).insert(lineEdit()->cursorPosition(), text);
    if(potential_text.contains(m_real_regex)) {
      lineEdit()->setText(potential_text);
    }
  }
}
