#include "Spire/AccountViewer/EntitlementEntryWidget.hpp"
#include <QToolButton>
#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/LegacyUI/ReadOnlyCheckBox.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_EntitlementEntryWidget.h"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;
using namespace std;

namespace {
  enum {
    MARKET_COLUMN = 0,
    BBO_COLUMN = 1,
    MARKET_QUOTES_COLUMN = 2,
    BOOK_QUOTES_COLUMN = 3,
    TIME_AND_SALES_COLUMN = 4,
    IMBALANCES_COLUMN = 5,
  };

  QHBoxLayout* MakeCheckboxLayout(bool checked) {
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
      QSizePolicy::Minimum));
    QToolButton* checkBox = new QToolButton();
    QIcon icon;
    if(checked) {
      icon.addFile(":/icons/check_small.png", QSize(), QIcon::Normal,
        QIcon::Off);
      icon.addFile(":/icons/check_small.png", QSize(), QIcon::Disabled,
        QIcon::Off);
    }
    checkBox->setIcon(icon);
    checkBox->setDisabled(true);
    checkBox->setAutoRaise(true);
    layout->addWidget(checkBox);
    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
      QSizePolicy::Minimum));
    return layout;
  }
}

EntitlementEntryWidget::EntitlementEntryWidget(Ref<UserProfile> userProfile,
    bool isReadOnly, const EntitlementDatabase::Entry& entitlement,
    std::shared_ptr<AccountEntitlementModel> model, QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_EntitlementEntryWidget>()),
      m_userProfile(userProfile.Get()),
      m_isReadOnly(isReadOnly),
      m_entitlement(entitlement),
      m_model(model) {
  m_ui->setupUi(this);
  m_ui->m_entitlementLabel->setText(
    QString::fromStdString(m_entitlement.m_name));
  if(m_entitlement.m_price == Money::ZERO) {
    m_ui->m_priceLabel->setText(tr("Free"));
  } else {
    const CurrencyDatabase::Entry& currency =
      m_userProfile->GetCurrencyDatabase().FromId(m_entitlement.m_currency);
    string price = currency.m_sign + lexical_cast<string>(entitlement.m_price) +
      " " + currency.m_code.GetData();
    m_ui->m_priceLabel->setText(QString::fromStdString(price));
  }
  m_ui->m_entitlementLabel->setChecked(
    m_model->HasEntitlement(m_entitlement.m_groupEntry));
  m_connections.AddConnection(
    m_model->ConnectEntitlementGrantedSignal(
      std::bind(&EntitlementEntryWidget::OnEntitlementGranted, this,
      std::placeholders::_1)));
  m_connections.AddConnection(
    m_model->ConnectEntitlementRevokedSignal(
      std::bind(&EntitlementEntryWidget::OnEntitlementRevoked, this,
      std::placeholders::_1)));
  if(m_isReadOnly) {
    m_isChecked = m_ui->m_entitlementLabel->isChecked();
    connect(m_ui->m_entitlementLabel, &QCheckBox::clicked, this,
      &EntitlementEntryWidget::OnEntitlementClicked);
  } else {
    connect(m_ui->m_entitlementLabel, &QCheckBox::stateChanged, this,
      &EntitlementEntryWidget::OnEntitlementChecked);
  }
  m_connections.AddConnection(m_ui->m_expandButton->ConnectExpandedSignal(
    std::bind(&EntitlementEntryWidget::OnTableExpanded, this)));
  m_connections.AddConnection(m_ui->m_expandButton->ConnectCollapsedSignal(
    std::bind(&EntitlementEntryWidget::OnTableCollapsed, this)));
  int row = 1;
  for(const pair<EntitlementKey, MarketDataTypeSet>& entitlement :
      m_entitlement.m_applicability) {
    MarketCode code;
    if(entitlement.first.m_market == MarketCode()) {
      code = entitlement.first.m_source;
    } else {
      code = entitlement.first.m_market;
    }
    const MarketDatabase::Entry& market =
      m_userProfile->GetMarketDatabase().FromCode(code);
    QLabel* marketLabel = new QLabel(
      QString::fromStdString(market.m_displayName));
    m_ui->m_applicabilityTableLayout->addWidget(marketLabel, row,
      MARKET_COLUMN);
    MarketDataTypeSet applicability = entitlement.second;
    QHBoxLayout* bboCheckBox = MakeCheckboxLayout(
      applicability.Test(MarketDataType::BBO_QUOTE));
    m_ui->m_applicabilityTableLayout->addLayout(bboCheckBox, row,
      BBO_COLUMN);
    QHBoxLayout* marketQuoteCheckBox = MakeCheckboxLayout(
      applicability.Test(MarketDataType::MARKET_QUOTE));
    m_ui->m_applicabilityTableLayout->addLayout(marketQuoteCheckBox, row,
      MARKET_QUOTES_COLUMN);
    QHBoxLayout* bookQuoteCheckBox = MakeCheckboxLayout(
      applicability.Test(MarketDataType::BOOK_QUOTE));
    m_ui->m_applicabilityTableLayout->addLayout(bookQuoteCheckBox, row,
      BOOK_QUOTES_COLUMN);
    QHBoxLayout* timeAndSalesCheckBox = MakeCheckboxLayout(
      applicability.Test(MarketDataType::TIME_AND_SALE));
    m_ui->m_applicabilityTableLayout->addLayout(timeAndSalesCheckBox, row,
      TIME_AND_SALES_COLUMN);
    QHBoxLayout* imbalancesCheckBox = MakeCheckboxLayout(
      applicability.Test(MarketDataType::ORDER_IMBALANCE));
    m_ui->m_applicabilityTableLayout->addLayout(imbalancesCheckBox, row,
      IMBALANCES_COLUMN);
    ++row;
  }
  m_ui->m_applicabilityTable->hide();
}

EntitlementEntryWidget::~EntitlementEntryWidget() {}

void EntitlementEntryWidget::OnEntitlementGranted(const DirectoryEntry& entry) {
  if(entry != m_entitlement.m_groupEntry) {
    return;
  }
  m_ui->m_entitlementLabel->blockSignals(true);
  m_ui->m_entitlementLabel->setChecked(true);
  m_ui->m_entitlementLabel->blockSignals(false);
}

void EntitlementEntryWidget::OnEntitlementRevoked(const DirectoryEntry& entry) {
  if(entry != m_entitlement.m_groupEntry) {
    return;
  }
  m_ui->m_entitlementLabel->blockSignals(true);
  m_ui->m_entitlementLabel->setChecked(false);
  m_ui->m_entitlementLabel->blockSignals(false);
}

void EntitlementEntryWidget::OnEntitlementChecked(int checkState) {
  if(checkState == Qt::Checked) {
    m_model->Grant(m_entitlement.m_groupEntry);
  } else {
    m_model->Revoke(m_entitlement.m_groupEntry);
  }
  m_ui->m_entitlementLabel->blockSignals(true);
  m_ui->m_entitlementLabel->setChecked(
    m_model->HasEntitlement(m_entitlement.m_groupEntry));
  m_ui->m_entitlementLabel->blockSignals(false);
}

void EntitlementEntryWidget::OnEntitlementClicked() {
  m_ui->m_entitlementLabel->setChecked(
    m_model->HasEntitlement(m_entitlement.m_groupEntry));
}

void EntitlementEntryWidget::OnTableExpanded() {
  m_ui->m_applicabilityTable->show();
}

void EntitlementEntryWidget::OnTableCollapsed() {
  m_ui->m_applicabilityTable->hide();
}
