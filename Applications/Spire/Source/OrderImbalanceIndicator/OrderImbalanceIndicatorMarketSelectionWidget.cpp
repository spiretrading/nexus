#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorMarketSelectionWidget.hpp"
#include <QCheckBox>
#include <QGridLayout>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const int MARKETS_PER_COLUMN = 6;
}

OrderImbalanceIndicatorMarketSelectionWidget::
    OrderImbalanceIndicatorMarketSelectionWidget(QWidget* parent)
    : QWidget(parent) {}

OrderImbalanceIndicatorMarketSelectionWidget::
    OrderImbalanceIndicatorMarketSelectionWidget(
    const MarketDatabase& marketDatabase,
    Ref<OrderImbalanceIndicatorModel> model, QWidget* parent)
    : QWidget(parent) {
  Initialize(marketDatabase, Ref(model));
}

void OrderImbalanceIndicatorMarketSelectionWidget::Initialize(
    const MarketDatabase& marketDatabase,
    Ref<OrderImbalanceIndicatorModel> model) {
  m_model = model.Get();
  QGridLayout* layout = new QGridLayout();
  setLayout(layout);
  int count = 0;
  for(const MarketDatabase::Entry& market : marketDatabase.GetEntries()) {
    int column = count / MARKETS_PER_COLUMN;
    int row = count % MARKETS_PER_COLUMN;
    std::unique_ptr<QCheckBox> checkBox = std::make_unique<QCheckBox>(
      QString::fromStdString(market.m_displayName));
    if(m_model->GetProperties().IsFiltered(market.m_code)) {
      checkBox->setCheckState(Qt::Unchecked);
    } else {
      checkBox->setCheckState(Qt::Checked);
    }
    m_markets[checkBox.get()] = market.m_code;
    layout->addWidget(checkBox.get(), row, column, 1, 1);
    connect(checkBox.get(), &QCheckBox::stateChanged, this,
      &OrderImbalanceIndicatorMarketSelectionWidget::OnCheckBoxStateChanged);
    m_checkBoxes.push_back(std::move(checkBox));
    ++count;
  }
}

unique_ptr<WindowSettings> OrderImbalanceIndicatorMarketSelectionWidget::
    GetWindowSettings() const {
  return std::make_unique<
    OrderImbalanceIndicatorMarketSelectionWidgetWindowSettings>(*this);
}

void OrderImbalanceIndicatorMarketSelectionWidget::OnCheckBoxStateChanged(
    int state) {
  QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
  MarketCode market = m_markets.at(checkBox);
  if(state == Qt::Checked) {
    m_model->SetMarketFilter(market, false);
  } else {
    m_model->SetMarketFilter(market, true);
  }
}
