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
}

NumericInputWidget::NumericInputWidget(QWidget* parent)
    : QAbstractSpinBox(parent),
      m_validator(new QDoubleValidator(std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::max(), DEFAULT_DECIMAL_PLACES, this)),
      m_step("1") {
  set_stylesheet(false, false);
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
      auto validation_result = validate(current_text);
      if(validation_result == QValidator::Acceptable ||
          validation_result == QValidator::Intermediate) {
        QAbstractSpinBox::keyPressEvent(event);
      }
    }
  }
}

QAbstractSpinBox::StepEnabled NumericInputWidget::stepEnabled() const {
  return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
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
    //setValue(singleStep() * step);
    //lineEdit()->setCursorPosition(text().length());
    return;
  }
  //connect(lineEdit(), &QLineEdit::selectionChanged, this,
  //  &DecimalInputWidget::revert_cursor);
  //m_last_cursor_pos = lineEdit()->cursorPosition();
  //stepBy(step);
  //disconnect(lineEdit(), &QLineEdit::selectionChanged, this,
  //  &DecimalInputWidget::revert_cursor);
  //lineEdit()->setCursorPosition(text().length());
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

QValidator::State NumericInputWidget::validate(QString& text) {
  auto index = 0;
  return m_validator->validate(text, index);
}

void NumericInputWidget::on_editing_finished() {
  auto text = lineEdit()->text();
  text.remove(QRegExp("^[0]*"));
  text.remove(QRegExp("0+$"));
  text.remove(QRegExp("\\.$"));
  if(text.startsWith(m_locale.decimalPoint())) {
    lineEdit()->setText(lineEdit()->text().insert(0, "0"));
  } else if(text.isEmpty()) {
    lineEdit()->setText("0");
  } else {
    lineEdit()->setText(text);
  }
  Q_EMIT editingFinished();
}
