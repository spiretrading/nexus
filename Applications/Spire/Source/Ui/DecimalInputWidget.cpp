#include "Spire/Ui/DecimalInputWidget.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto DEFAULT_DECIMAL_PLACES = 6;
  const auto SHIFT_STEPS = 10;

  auto ARROW_WIDTH() {
    static auto width = scale_width(6);
    return width;
  }

  auto BUTTON_PADDING() {
    static auto padding = scale_width(17);
    return padding;
  }
}

DecimalInputWidget::DecimalInputWidget(double value, QWidget* parent)
    : QDoubleSpinBox(parent),
      m_last_valid_value(value),
      m_last_cursor_pos(0),
      m_has_first_click(false) {
  setValue(m_last_valid_value);
  setDecimals(DEFAULT_DECIMAL_PLACES);
  setStyleSheet(QString(R"(
    QDoubleSpinBox {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %5px;
      padding-left: %4px;
    }

    QDoubleSpinBox:focus {
      border: %1px solid #4B23A0;
    }

    QDoubleSpinBox::up-button {
      border: none;
    }

    QDoubleSpinBox::down-button {
      border: none;
    }

    QDoubleSpinBox::up-arrow {
      height: %2px;
      image: url(:/Icons/arrow-up.svg);
      padding-top: %6px;
      width: %3px;
    }

    QDoubleSpinBox::down-arrow {
      height: %2px;
      image: url(:/Icons/arrow-down.svg);
      width: %3px;
    })").arg(scale_width(1)).arg(scale_height(6)).arg(ARROW_WIDTH())
        .arg(scale_width(10)).arg(scale_height(12)).arg(scale_height(4)));
  setContextMenuPolicy(Qt::NoContextMenu);
  connect(this, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
    &DecimalInputWidget::on_value_changed);
  connect(lineEdit(), &QLineEdit::textEdited, this,
    &DecimalInputWidget::on_text_edited);
  lineEdit()->installEventFilter(this);
}

QString DecimalInputWidget::textFromValue(double value) const {
  if(value == 0.0) {
    return "0";
  }
  QString str = QString::number(value, 'f', decimals());
  str.remove(QRegExp("0+$"));
  str.remove(QRegExp("\\.$"));
  return str;
}

connection DecimalInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection DecimalInputWidget::connect_submit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool DecimalInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == lineEdit() && event->type() == QEvent::MouseButtonPress &&
      !m_has_first_click) {
    auto e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton) {
      m_has_first_click = true;
      selectAll();
      return true;
    }
  }
  return QDoubleSpinBox::eventFilter(watched, event);
}

void DecimalInputWidget::focusInEvent(QFocusEvent* event) {
  if(event->reason() != Qt::MouseFocusReason) {
    m_has_first_click = true;
  }
  QDoubleSpinBox::focusInEvent(event);
}

void DecimalInputWidget::focusOutEvent(QFocusEvent* event) {
  m_has_first_click = false;
  if(text().isEmpty()) {
    blockSignals(true);
    setValue(m_last_valid_value);
    blockSignals(false);
  }
  QDoubleSpinBox::focusOutEvent(event);
  m_last_valid_value = value();
  m_submit_signal(m_last_valid_value);
}

void DecimalInputWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Delete:
      setValue(m_last_valid_value);
      return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(text().isEmpty()) {
        blockSignals(true);
        setValue(m_last_valid_value);
        blockSignals(false);
      }
      lineEdit()->setText(textFromValue(value()));
      m_last_valid_value = value();
      m_submit_signal(m_last_valid_value);
      return;
    case Qt::Key_Up:
      add_step(1, event->modifiers());
      return;
    case Qt::Key_Down:
      add_step(-1, event->modifiers());
      return;
  }
  QDoubleSpinBox::keyPressEvent(event);
}

void DecimalInputWidget::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(event->x() > width() - BUTTON_PADDING()) {
      if(event->y() < height() / 2) {
        add_step(1, event->modifiers());
      } else {
        add_step(-1, event->modifiers());
      }
      return;
    } else if(!m_has_first_click) {
      selectAll();
      m_has_first_click = true;
    }
  }
  QDoubleSpinBox::mousePressEvent(event);
}

void DecimalInputWidget::add_step(int step, Qt::KeyboardModifiers modifiers) {
  step = [&] {
    if(modifiers == Qt::ShiftModifier) {
      return step * SHIFT_STEPS;
    }
    return step;
  }();
  if(text().isEmpty()) {
    setValue(singleStep() * step);
    lineEdit()->setCursorPosition(text().length());
    return;
  }
  connect(lineEdit(), &QLineEdit::selectionChanged, this,
    &DecimalInputWidget::revert_cursor);
  m_last_cursor_pos = lineEdit()->cursorPosition();
  stepBy(step);
  disconnect(lineEdit(), &QLineEdit::selectionChanged, this,
    &DecimalInputWidget::revert_cursor);
  lineEdit()->setCursorPosition(text().length());
}

void DecimalInputWidget::revert_cursor() {
  lineEdit()->blockSignals(true);
  lineEdit()->deselect();
  lineEdit()->setCursorPosition(m_last_cursor_pos);
  lineEdit()->blockSignals(false);
}

void DecimalInputWidget::on_text_edited(const QString& text) {
  if(minimum() >= 0.0 && text.contains("-")) {
    lineEdit()->blockSignals(true);
    lineEdit()->setText(lineEdit()->text().remove("-"));
    lineEdit()->blockSignals(false);
  }
}

void DecimalInputWidget::on_value_changed(double value) {
  m_change_signal(value);
}
