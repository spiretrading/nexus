#include "Spire/Ui/ComboBoxEditor.hpp"
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>

using namespace Spire;

ComboBoxEditor::ComboBoxEditor(StaticDropDownMenu* menu, QWidget* parent)
    : QLineEdit(parent),
      m_menu(menu),
      m_last_key(Qt::Key_unknown) {
  setReadOnly(true);
  setFocusProxy(m_menu);
  m_menu->findChild<DropDownMenuList*>()->setFocusProxy(m_menu);
  m_menu->set_style(StaticDropDownMenu::Style::CELL);
  m_menu->installEventFilter(this);
  m_menu->connect_value_selected_signal([=] (const auto& value) {
    Q_EMIT editingFinished();
  });
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
}

bool ComboBoxEditor::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    m_last_key = static_cast<Qt::Key>(e->key());
    if(e->key() == Qt::Key_Escape || e->key() == Qt::Key_Delete) {
      Q_EMIT editingFinished();
    } else if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
      qApp->sendEvent(this, e);
    }
  }
  return QLineEdit::eventFilter(watched, event);
}

void ComboBoxEditor::showEvent(QShowEvent* event) {
  m_menu->findChild<DropDownMenuList*>()->show();
  QLineEdit::showEvent(event);
}

Qt::Key ComboBoxEditor::get_last_key() const {
  return m_last_key;
}

QVariant ComboBoxEditor::get_value() const {
  return m_menu->get_current_input_item();
}

void ComboBoxEditor::set_value(const QVariant& value) {
  m_menu->set_current_item(value);
}
