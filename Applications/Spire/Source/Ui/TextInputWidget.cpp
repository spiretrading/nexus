#include "Spire/Ui/TextInputWidget.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

TextInputWidget::TextInputWidget(QWidget* parent)
  : TextInputWidget({}, parent) {}

TextInputWidget::TextInputWidget(QString text, QWidget* parent)
    : QLineEdit(std::move(text), parent) {
  setContextMenuPolicy(Qt::NoContextMenu);
  set_default_style();
}

void TextInputWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
      return;
    case Qt::Key_Delete:
      setText("");
      Q_EMIT editingFinished();
      return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      Q_EMIT editingFinished();
      return;
  }
  QLineEdit::keyPressEvent(event);
}

void TextInputWidget::set_cell_style() {
  setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    color: #000000;
    font-family: Roboto;
    font-size: %1px;
    padding-left: %2px;
  )").arg(scale_height(12)).arg(scale_width(5)));
}

void TextInputWidget::set_default_style() {
  setStyleSheet(QString(R"(
    QLineEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding-left: %4px;
    }

    QLineEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12))
        .arg(scale_width(6)));
}
