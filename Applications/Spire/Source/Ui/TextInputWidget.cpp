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

void TextInputWidget::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Delete) {
    setText("");
    Q_EMIT editingFinished();
    return;
  }
  QLineEdit::keyPressEvent(event);
}
