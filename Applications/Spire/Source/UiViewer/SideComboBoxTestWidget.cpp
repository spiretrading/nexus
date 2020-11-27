#include "Spire/UiViewer/SideComboBoxTestWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SideComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideComboBoxTestWidget::SideComboBoxTestWidget(QWidget* parent)
    : ComboBoxAdapter(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto combo_box = new SideComboBox(this);
  combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(combo_box);
  connect(combo_box, &SideComboBox::editingFinished, [=] {
    m_selected_signal(QVariant::fromValue(combo_box->get_side()));
  });
}

connection SideComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
