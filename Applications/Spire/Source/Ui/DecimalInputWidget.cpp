#include "Spire/Ui/DecimalInputWidget.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

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
    })").arg(scale_width(1)).arg(scale_height(6)).arg(scale_width(6))
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
      connect(lineEdit(), &QLineEdit::selectionChanged, this,
        &DecimalInputWidget::revert_cursor);
      m_last_cursor_pos = lineEdit()->cursorPosition();
      stepBy(1);
      disconnect(lineEdit(), &QLineEdit::selectionChanged, this,
        &DecimalInputWidget::revert_cursor);
      return;
    case Qt::Key_Down:
      connect(lineEdit(), &QLineEdit::selectionChanged, this,
        &DecimalInputWidget::revert_cursor);
      m_last_cursor_pos = lineEdit()->cursorPosition();
      stepBy(-1);
      disconnect(lineEdit(), &QLineEdit::selectionChanged, this,
        &DecimalInputWidget::revert_cursor);
      return;
  }
  QDoubleSpinBox::keyPressEvent(event);
}

void DecimalInputWidget::on_editing_finished() {
  //lineEdit()->deselect();
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
