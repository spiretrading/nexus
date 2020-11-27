#include "Spire/UiViewer/MarketComboBoxTestWidget.hpp"
#include <QHBoxLayout>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/Ui/MarketComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

MarketComboBoxTestWidget::MarketComboBoxTestWidget(QWidget* parent)
    : ComboBoxAdapter(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto combo_box = new MarketComboBox(GetDefaultMarketDatabase(), this);
  combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(combo_box);
  connect(combo_box, &MarketComboBox::editingFinished, [=] {
    m_selected_signal(QVariant::fromValue<MarketToken>(
      combo_box->get_market()));
  });
}

connection MarketComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
