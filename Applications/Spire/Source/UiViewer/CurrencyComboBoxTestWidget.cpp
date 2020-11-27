#include "Spire/UiViewer/CurrencyComboBoxTestWidget.hpp"
#include <QHBoxLayout>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CurrencyComboBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CurrencyComboBoxTestWidget::CurrencyComboBoxTestWidget(QWidget* parent)
    : ComboBoxAdapter(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto database = GetDefaultCurrencyDatabase();
  auto combo_box = new CurrencyComboBox(database, this);
  combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(combo_box);
  connect(combo_box, &CurrencyComboBox::editingFinished, [=] {
    m_selected_signal(QVariant::fromValue(combo_box->get_currency()));
  });
}

connection CurrencyComboBoxTestWidget::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}
