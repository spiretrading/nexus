#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include <QEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

InputFieldEditor::InputFieldEditor(std::vector<QString> items,
    int list_width, QWidget* parent)
    : QLineEdit(parent),
      m_list_width(list_width) {
  m_menu_list = new DropDownMenuList(items, this);
  m_menu_list->hide();
  m_menu_list->connect_selected_signal([=] (auto item) {
    on_item_selected(item);
  });
  window()->installEventFilter(this);
}

bool InputFieldEditor::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_menu_list->isVisible()) {
        move_menu_list();
      }
    }
  }
  return QLineEdit::eventFilter(watched, event);
}

void InputFieldEditor::showEvent(QShowEvent* event) {
  on_clicked();
}

void InputFieldEditor::move_menu_list() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x() - scale_width(8);
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_menu_list->move(x_pos, y_pos + 2);
  m_menu_list->raise();
}

void InputFieldEditor::on_clicked() {
  move_menu_list();
  m_menu_list->setFixedWidth(m_list_width);
  m_menu_list->show();
  m_menu_list->raise();
}

void InputFieldEditor::on_item_selected(const QString& text) {
  m_menu_list->hide();
  emit editingFinished();
}
