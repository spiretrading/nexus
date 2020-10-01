#include "Spire/Ui/OrderTypeComboBox.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeComboBox::OrderTypeComboBox(QWidget* parent)
  : OrderTypeComboBox(false, parent) {}

OrderTypeComboBox::OrderTypeComboBox(bool is_cell_widget, QWidget* parent)
    : QLineEdit(parent),
      m_is_cell_widget(is_cell_widget) {
  setReadOnly(true);
  auto items = [] {
    auto types = std::vector<QVariant>();
    types.reserve(OrderType::COUNT);
    for(auto type : Beam::MakeRange<OrderType>()) {
      types.push_back(QVariant::fromValue(type));
    }
    return types;
  }();
  m_menu = new StaticDropDownMenu(items, this);
  setFocusProxy(m_menu);
  if(m_is_cell_widget) {
    m_menu->set_style(StaticDropDownMenu::Style::CELL);
    m_menu->installEventFilter(this);
  }
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_menu);
  m_value_connection = m_menu->connect_value_selected_signal(
    [=] (const auto& value) {
      m_selected_signal(value.value<OrderType>());
      Q_EMIT editingFinished();
    });
}

bool OrderTypeComboBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Escape) {
      Q_EMIT editingFinished();
    } else if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
      qApp->sendEvent(this, e);
    }
  }
  return QLineEdit::eventFilter(watched, event);
}

void OrderTypeComboBox::showEvent(QShowEvent* event) {
  if(m_is_cell_widget) {
    m_menu->findChild<DropDownMenuList*>()->show();
  }
  QLineEdit::showEvent(event);
}

OrderType OrderTypeComboBox::get_order_type() const {
  if(m_is_cell_widget) {
    return m_menu->get_current_input_item().value<OrderType>();
  }
  return m_menu->get_current_item().value<OrderType>();
}

void OrderTypeComboBox::set_order_type(OrderType type) {
  m_menu->set_current_item(QVariant::fromValue(type));
}

connection OrderTypeComboBox::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
