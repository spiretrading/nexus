#include "Spire/KeyBindings/NameInputEditor.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

NameInputEditor::NameInputEditor(QWidget* parent)
    : QLineEdit(parent) {
  setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    color: #000000;
    font-family: Roboto;
    font-size: %1px;
    padding-left: %2px;
  )").arg(scale_height(12)).arg(scale_width(5)));
  setContextMenuPolicy(Qt::NoContextMenu);
}

void NameInputEditor::keyPressEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_Up || key == Qt::Key_Down) {
    return;
  }
  if(key == Qt::Key_Delete) {
    setText("");
    emit editingFinished();
    return;
  } else if(key == Qt::Key_Enter || key == Qt::Key_Return) {
    emit editingFinished();
    return;
  }
  QLineEdit::keyPressEvent(event);
}
