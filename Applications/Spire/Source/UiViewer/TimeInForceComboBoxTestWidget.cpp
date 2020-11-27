#include "Spire/UiViewer/TimeInForceComboBoxTestWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TimeInForceComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeInForceComboBoxTestWidget::TimeInForceComboBoxTestWidget(QWidget* parent)
    : ComboBoxAdapter(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto combo_box = new TimeInForceComboBox(this);
  combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(combo_box);
  connect(combo_box, &TimeInForceComboBox::editingFinished, [=] {
    m_selected_signal(QVariant::fromValue(combo_box->get_time_in_force()));
  });
}

connection TimeInForceComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
