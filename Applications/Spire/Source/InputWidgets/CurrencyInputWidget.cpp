#include "Spire/InputWidgets/CurrencyInputWidget.hpp"
#include <QComboBox>
#include <QVBoxLayout>
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

CurrencyInputWidget::CurrencyInputWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_userProfile{nullptr},
      m_isReadOnly{false} {
  auto layout = new QVBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_currencyComboBox = new QComboBox{this};
  layout->addWidget(m_currencyComboBox);
  connect(m_currencyComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
    &CurrencyInputWidget::OnCurrencyActivated);
}

CurrencyInputWidget::CurrencyInputWidget(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : CurrencyInputWidget{parent, flags} {
  Initialize(Ref(userProfile));
}

CurrencyInputWidget::~CurrencyInputWidget() {}

void CurrencyInputWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.Get();
  auto& currencies = m_userProfile->GetCurrencyDatabase();
  for(auto& currency : currencies.GetEntries()) {
    m_currencyComboBox->addItem(QString::fromStdString(
      currency.m_code.GetData()));
  }
}

CurrencyId CurrencyInputWidget::GetCurrency() const {
  if(m_currencyComboBox->count() == 0) {
    return CurrencyId::NONE;
  }
  return m_userProfile->GetCurrencyDatabase().GetEntries()[
    m_currencyComboBox->currentIndex()].m_id;
}

void CurrencyInputWidget::SetCurrency(CurrencyId currency) {
  auto& currencies = m_userProfile->GetCurrencyDatabase();
  for(auto i = size_t{0}; i < currencies.GetEntries().size(); ++i) {
    if(currencies.GetEntries()[i].m_id == currency) {
      m_currencyComboBox->setCurrentIndex(i);
      break;
    }
  }
}

void CurrencyInputWidget::SetReadOnly(bool value) {
  m_isReadOnly = value;
  m_currencyComboBox->setEnabled(!value);
}

connection CurrencyInputWidget::ConnectCurrencyUpdatedSignal(
    const CurrencyUpdatedSignal::slot_type& slot) const {
  return m_currencyUpdatedSignal.connect(slot);
}

void CurrencyInputWidget::OnCurrencyActivated(int index) {
  auto currency = m_userProfile->GetCurrencyDatabase().GetEntries()[index].m_id;
  m_currencyUpdatedSignal(currency);
}
