#include "Spire/Ui/DecimalInputWidget.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
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
      m_last_cursor_pos(0) {
  setValue(value);
  setDecimals(std::numeric_limits<double>::digits10 - 1);
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

void DecimalInputWidget::focusInEvent(QFocusEvent* event) {
  QDoubleSpinBox::focusInEvent(event);
  switch(event->reason()) {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
      lineEdit()->selectAll();
  }
}

void DecimalInputWidget::focusOutEvent(QFocusEvent* event) {
  QDoubleSpinBox::focusOutEvent(event);
  m_submit_signal(value());
}

void DecimalInputWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Delete:
      setValue(std::max(0.0, minimum()));
      return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(text().isEmpty()) {
        blockSignals(true);
        setValue(0);
        blockSignals(false);
      }
      m_submit_signal(value());
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
  if(event->x() > width() - BUTTON_PADDING()) {
    if(event->y() < height() / 2) {
      add_step(1, event->modifiers());
    } else {
      add_step(-1, event->modifiers());
    }
    return;
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
    return;
  }
  connect(lineEdit(), &QLineEdit::selectionChanged, this,
    &DecimalInputWidget::revert_cursor);
  m_last_cursor_pos = lineEdit()->cursorPosition();
  stepBy(step);
  disconnect(lineEdit(), &QLineEdit::selectionChanged, this,
    &DecimalInputWidget::revert_cursor);
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
