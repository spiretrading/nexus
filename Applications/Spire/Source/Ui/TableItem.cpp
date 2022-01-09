#include "Spire/Ui/TableItem.hpp"
#include <QHBoxLayout>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableItem::TableItem(QWidget& component, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_button = new Button(&component);
  layout->addWidget(m_button);
}

connection TableItem::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_button->connect_clicked_signal(slot);
}
