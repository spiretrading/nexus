#include "Spire/UiViewer/OrderTypeComboBoxTestWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/OrderTypeComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeComboBoxTestWidget::OrderTypeComboBoxTestWidget(QWidget* parent)
    : ComboBoxAdapter(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto combo_box = new OrderTypeComboBox(this);
  layout->addWidget(combo_box);
  combo_box->connect_selected_signal([=] (auto value) {
    m_selected_signal(QVariant::fromValue(value));
  });
}

connection OrderTypeComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
