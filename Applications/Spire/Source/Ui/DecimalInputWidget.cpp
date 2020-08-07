#include "Spire/Ui/DecimalInputWidget.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

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
      m_value(value),
      m_last_cursor_pos(0) {
  setDecimals(std::numeric_limits<double>::digits10 - 1);
  //setMinimum(std::numeric_limits<double>::lowest());
  //setMaximum(std::numeric_limits<double>::max());
  setMinimum(0);
  setMaximum(99);
  setValue(m_value);
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
      background-color: red;
    }

    QDoubleSpinBox::down-button {
      border: none;
      background-color: red;
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
  //connect(this, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
  //  &DecimalInputWidget::on_value_changed);
  connect(lineEdit(), &QLineEdit::textEdited, this,
    &DecimalInputWidget::on_text_edited);
  connect(this, &QDoubleSpinBox::editingFinished, this,
    &DecimalInputWidget::on_editing_finished);
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

void DecimalInputWidget::focusInEvent(QFocusEvent* event) {
  QDoubleSpinBox::focusInEvent(event);
  switch(event->reason()) {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
      lineEdit()->selectAll();
  }
}

void DecimalInputWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      Q_EMIT valueChanged(value());
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

void DecimalInputWidget::on_editing_finished() {
  //lineEdit()->deselect();
}

void DecimalInputWidget::add_step(int step, Qt::KeyboardModifiers modifiers) {
  connect(lineEdit(), &QLineEdit::selectionChanged, this,
    &DecimalInputWidget::revert_cursor);
  m_last_cursor_pos = lineEdit()->cursorPosition();
  step = [&] {
    if(modifiers == Qt::ShiftModifier) {
      return step * SHIFT_STEPS;
    }
    return step;
  }();
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
  //m_last_cursor_pos = lineEdit()->cursorPosition();
  //lineEdit()->deselect();
}
