#include "Spire/Ui/RealSpinBox.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QStyleOption>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto SHIFT_STEPS = 10;
  const auto MOUSE_REPEAT_DELAY_MS = 20;
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

  auto clamp(RealSpinBox::Real value, RealSpinBox::Real min,
      RealSpinBox::Real max) {
    return std::clamp(value, min, max, [] (const RealSpinBox::Real& a,
        const RealSpinBox::Real& b) {
      return a.compare(b) == -1;
    });
  }
}

RealSpinBox::RealSpinBox(Real value, QWidget* parent)
    : QAbstractSpinBox(parent),
      m_minimum_decimals(0),
      m_minimum(std::numeric_limits<Real>::lowest()),
      m_maximum(std::numeric_limits<Real>::max()),
      m_step(1),
      m_last_valid_value(value),
      m_has_first_click(false),
      m_up_arrow_timer_id(-1),
      m_down_arrow_timer_id(-1) {
  connect(lineEdit(), &QLineEdit::textChanged, this,
    &RealSpinBox::on_text_changed);
  setContextMenuPolicy(Qt::NoContextMenu);
  lineEdit()->setText(display_string(m_last_valid_value));
  set_decimal_places(MAXIMUM_DECIMAL_PLACES);
  lineEdit()->installEventFilter(this);
}

void RealSpinBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_stylesheet();
  }
  QAbstractSpinBox::changeEvent(event);
}

bool RealSpinBox::eventFilter(QObject* watched, QEvent* event) {
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

void RealSpinBox::focusInEvent(QFocusEvent* event) {
  if(event->reason() != Qt::MouseFocusReason) {
    m_has_first_click = true;
  }
  QAbstractSpinBox::focusInEvent(event);
}

void RealSpinBox::focusOutEvent(QFocusEvent* event) {
  m_has_first_click = false;
  blockSignals(true);
  QAbstractSpinBox::focusOutEvent(event);
  blockSignals(false);
  on_editing_finished();
}

void RealSpinBox::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers().testFlag(Qt::ControlModifier)) {
    QAbstractSpinBox::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
    case Qt::Key_Backspace:
      lineEdit()->backspace();
      return;
    case Qt::Key_Delete:
      lineEdit()->clear();
      return;
    case Qt::Key_Escape:
      lineEdit()->setText(display_string(m_last_valid_value));
      Q_EMIT editingFinished();
      return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      on_editing_finished();
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

void RealSpinBox::mouseMoveEvent(QMouseEvent* event) {
  auto control = get_current_control(event->pos());
  if(m_up_arrow_timer_id != -1 &&
      control != QStyle::SubControl::SC_ScrollBarAddLine) {
    stop_timer(m_up_arrow_timer_id);
  } else if(m_down_arrow_timer_id != -1 &&
      control != QStyle::SubControl::SC_ScrollBarSubLine) {
    stop_timer(m_down_arrow_timer_id);
  }
  QAbstractSpinBox::mouseMoveEvent(event);
}

void RealSpinBox::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && !m_has_first_click) {
    selectAll();
    m_has_first_click = true;
  }
  auto control = get_current_control(event->pos());
  if(control == QStyle::SubControl::SC_ScrollBarAddLine) {
    m_up_arrow_timer_id = startTimer(qApp->keyboardInputInterval());
  } else if(control == QStyle::SubControl::SC_ScrollBarSubLine) {
    m_down_arrow_timer_id = startTimer(qApp->keyboardInputInterval());
  }
  QAbstractSpinBox::mousePressEvent(event);
}

void RealSpinBox::mouseReleaseEvent(QMouseEvent* event) {
  stop_timer(m_up_arrow_timer_id);
  stop_timer(m_down_arrow_timer_id);
  QAbstractSpinBox::mouseReleaseEvent(event);
}

void RealSpinBox::timerEvent(QTimerEvent* event) {
  if(event->timerId() == m_up_arrow_timer_id) {
    stop_timer(m_up_arrow_timer_id);
    m_up_arrow_timer_id = startTimer(MOUSE_REPEAT_DELAY_MS);
    add_step(1, qApp->keyboardModifiers());
  } else if(event->timerId() == m_down_arrow_timer_id) {
    stop_timer(m_down_arrow_timer_id);
    m_down_arrow_timer_id = startTimer(MOUSE_REPEAT_DELAY_MS);
    add_step(-1, qApp->keyboardModifiers());
  }
}

QAbstractSpinBox::StepEnabled RealSpinBox::stepEnabled() const {
  return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
}

void RealSpinBox::set_decimal_places(int decimals) {
  m_decimals = std::min(decimals, MAXIMUM_DECIMAL_PLACES);
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

void RealSpinBox::set_minimum_decimal_places(int decimals) {
  m_minimum_decimals = decimals;
  lineEdit()->setText(display_string(m_last_valid_value));
}

void RealSpinBox::set_minimum(Real minimum) {
  m_minimum = minimum;
  if(auto value = get_value(text()); value && value->compare(m_minimum) < 0) {
    m_last_valid_value = m_minimum;
    lineEdit()->setText(display_string(m_minimum));
  }
  update_stylesheet();
}

void RealSpinBox::set_maximum(Real maximum) {
  m_maximum = maximum;
  if(auto value = get_value(text()); value && value->compare(m_maximum) > 0) {
    m_last_valid_value = m_maximum;
    lineEdit()->setText(display_string(m_maximum));
  }
  update_stylesheet();
}

void RealSpinBox::set_step(Real step) {
  m_step = step;
}

RealSpinBox::Real RealSpinBox::get_value() const {
  return m_last_valid_value;
}

void RealSpinBox::set_value(Real value) {
  blockSignals(true);
  value = clamp(value, m_minimum, m_maximum);
  lineEdit()->setText(display_string(value));
  blockSignals(false);
}

void RealSpinBox::stepBy(int step) {
  add_step(step);
}

connection RealSpinBox::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

void RealSpinBox::add_step(int step) {
  add_step(step, Qt::NoModifier);
}

void RealSpinBox::add_step(int step, Qt::KeyboardModifiers modifiers) {
  if(modifiers == Qt::ShiftModifier) {
    step *= SHIFT_STEPS;
  }
  auto value = get_value(text());
  if(text().isEmpty() || !value) {
    value = 0;
  }
  auto stepped_value = m_step;
  stepped_value *= step;
  *value += stepped_value;
  value = clamp(*value, m_minimum, m_maximum);
  lineEdit()->setText(display_string(*value));
  lineEdit()->setCursorPosition(text().length());
}

QString RealSpinBox::display_string(Real value) {
  auto current_text = text();
  current_text.remove(QRegularExpression("0+$"));
  current_text.remove(QRegularExpression(
    QString("\\%1$").arg(m_locale.decimalPoint())));
  auto point_index = current_text.indexOf(m_locale.decimalPoint());
  if(point_index != -1 || m_minimum_decimals > 0) {
    auto decimal_places = [&] {
      if(point_index != -1) {
        return std::max(m_minimum_decimals,
          current_text.length() - point_index - 1);
      }
      return m_minimum_decimals;
    }();
    return QString::fromStdString(value.str(decimal_places,
      std::ios_base::fixed));
  }
  return QString::fromStdString(value.str(text().length(),
    std::ios_base::dec));
}

QStyle::SubControl RealSpinBox::get_current_control(const QPoint& mouse_pos) {
  auto opt = QStyleOptionSpinBox();
  initStyleOption(&opt);
  opt.subControls = QStyle::SC_All;
  return style()->hitTestComplexControl(QStyle::CC_SpinBox, &opt, mouse_pos,
    this);
}

boost::optional<RealSpinBox::Real> RealSpinBox::get_value(
    const QString& text) const {
  try {
    return Real(text.toStdString().c_str());
  } catch (const std::runtime_error&) {
    return boost::none;
  }
}

bool RealSpinBox::is_valid(const QString& text) {
  if(!text.contains(m_real_regex)) {
    return false;
  }
  if(!text.contains(QRegularExpression("[0-9]"))) {
    return true;
  }
  if(auto value = get_value(text)) {
    return value->compare(m_minimum) >= 0 && value->compare(m_maximum) <= 0;
  }
  return true;
}

void RealSpinBox::set_stylesheet(bool is_up_disabled, bool is_down_disabled) {
  if(isEnabled()) {
    setButtonSymbols(UpDownArrows);
  } else {
    setButtonSymbols(NoButtons);
  }
  auto [up_icon, up_icon_hover] = [&] {
    if(is_up_disabled) {
      return std::tuple(UP_ARROW_DISABLED_ICON, UP_ARROW_DISABLED_ICON);
    }
    return std::tuple(UP_ARROW_ICON, UP_ARROW_HOVER_ICON);
  }();
  auto [down_icon, down_icon_hover] = [&] {
    if(is_down_disabled) {
      return std::tuple(DOWN_ARROW_DISABLED_ICON, DOWN_ARROW_DISABLED_ICON);
    }
    return std::tuple(DOWN_ARROW_ICON, DOWN_ARROW_HOVER_ICON);
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
        .arg(scale_width(10)).arg(scale_height(12)).arg(scale_height(2))
        .arg(up_icon).arg(up_icon_hover).arg(down_icon).arg(down_icon_hover)
        .arg(UP_ARROW_DISABLED_ICON).arg(DOWN_ARROW_DISABLED_ICON));
}

void RealSpinBox::stop_timer(int& timer_id) {
  if(timer_id != -1) {
    killTimer(timer_id);
    timer_id = -1;
  }
}

void RealSpinBox::update_stylesheet() {
  if(auto value = get_value(text()); !value) {
    set_stylesheet(false, false);
  } else if(value->compare(m_minimum) == 0) {
    set_stylesheet(false, true);
  } else if(value->compare(m_maximum) == 0) {
    set_stylesheet(true, false);
  } else {
    set_stylesheet(false, false);
  }
}

void RealSpinBox::on_editing_finished() {
  if(auto value = get_value(text()); text().isEmpty() || !value) {
    lineEdit()->setText(display_string(m_last_valid_value));
  } else {
    m_last_valid_value = *value;
    lineEdit()->setText(display_string(m_last_valid_value));
  }
  Q_EMIT editingFinished();
}

void RealSpinBox::on_text_changed(const QString& text) {
  if(!is_valid(text)) {
    lineEdit()->blockSignals(true);
    lineEdit()->setText(m_last_valid_text);
    lineEdit()->blockSignals(false);
    return;
  }
  m_last_valid_text = text;
  update_stylesheet();
  if(auto value = get_value(m_last_valid_text)) {
    m_change_signal(*value);
  }
}
