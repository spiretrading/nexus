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
  auto database = GetDefaultMarketDatabase();
  auto combo_box = new MarketComboBox(database, this);
  layout->addWidget(combo_box);
  combo_box->connect_selected_signal([=] (auto value) {
    m_selected_signal(QVariant::fromValue<MarketToken>(value));
  });
}

connection MarketComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
