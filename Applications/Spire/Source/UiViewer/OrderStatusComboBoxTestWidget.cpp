#include "Spire/UiViewer/OrderStatusComboBoxTestWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/OrderStatusComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderStatusComboBoxTestWidget::OrderStatusComboBoxTestWidget(QWidget* parent)
    : ComboBoxAdapter(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto combo_box = new OrderStatusComboBox(this);
  combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(combo_box);
  connect(combo_box, &OrderStatusComboBox::editingFinished, [=] {
    m_selected_signal(QVariant::fromValue(combo_box->get_order_status()));
  });
}

connection OrderStatusComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
