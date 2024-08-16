#include "Spire/AccountViewer/RiskWidget.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/RiskModel.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_RiskWidget.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace std;

RiskWidget::RiskWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_RiskWidget>()),
      m_isReadOnly(true) {
  m_ui->setupUi(this);
  connect(m_ui->m_currencyInput,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &RiskWidget::OnCurrencyIndexChanged);
}

RiskWidget::~RiskWidget() {}

void RiskWidget::Initialize(Ref<UserProfile> userProfile,
    bool isReadOnly) {
  m_isReadOnly = isReadOnly;
  m_userProfile = userProfile.Get();
  m_ui->m_currencyInput->clear();
  if(!m_isReadOnly) {
    auto& currencies = m_userProfile->GetCurrencyDatabase().GetEntries();
    for(auto& currency : currencies) {
      m_ui->m_currencyInput->addItem(QString::fromStdString(
        currency.m_code.GetData()));
    }
  }
  m_ui->m_buyingPowerInput->setReadOnly(isReadOnly);
  m_ui->m_netLossInput->setReadOnly(isReadOnly);
  m_ui->m_transitionTimeInput->setReadOnly(isReadOnly);
}

const RiskModel& RiskWidget::GetModel() const {
  return *m_model;
}

RiskModel& RiskWidget::GetModel() {
  return *m_model;
}

void RiskWidget::SetModel(const std::shared_ptr<RiskModel>& model) {
  m_model = model;
  auto& currency = m_userProfile->GetCurrencyDatabase().FromId(
    m_model->GetRiskParameters().m_currency);
  if(m_isReadOnly) {
    m_ui->m_currencyInput->clear();
    m_ui->m_currencyInput->addItem(QString::fromStdString(
      currency.m_code.GetData()));
  } else {
    for(int i = 0; i < m_ui->m_currencyInput->count(); ++i) {
      if(m_ui->m_currencyInput->itemText(i).toStdString() == currency.m_code) {
        m_ui->m_currencyInput->setCurrentIndex(i);
        break;
      }
    }
  }
  m_ui->m_buyingPowerInput->setPrefix(QString::fromStdString(currency.m_sign));
  m_ui->m_buyingPowerInput->setValue(
    static_cast<double>(m_model->GetRiskParameters().m_buyingPower));
  m_ui->m_buyingPowerInput->setSuffix(QString::fromStdString(
    string{" "} + currency.m_code.GetData()));
  m_ui->m_netLossInput->setPrefix(QString::fromStdString(currency.m_sign));
  m_ui->m_netLossInput->setValue(
    static_cast<double>(m_model->GetRiskParameters().m_netLoss));
  m_ui->m_netLossInput->setSuffix(QString::fromStdString(
    string{" "} + currency.m_code.GetData()));
  QTime timeDisplay(0, 0, 0, 0);
  timeDisplay = timeDisplay.addMSecs(static_cast<int>(
    m_model->GetRiskParameters().m_transitionTime.total_milliseconds()));
  m_ui->m_transitionTimeInput->setTime(timeDisplay);
}

void RiskWidget::Commit() {
  auto& currency = m_userProfile->GetCurrencyDatabase().FromCode(
    m_ui->m_currencyInput->currentText().toStdString());
  m_model->GetRiskParameters().m_currency = currency.m_id;
  m_model->GetRiskParameters().m_buyingPower =
    Money{Quantity{m_ui->m_buyingPowerInput->value()}};
  m_model->GetRiskParameters().m_netLoss =
    Money{Quantity{m_ui->m_netLossInput->value()}};

  // TODO
  m_model->GetRiskParameters().m_lossFromTop = 1;
  auto transitionTime = m_ui->m_transitionTimeInput->time();
  m_model->GetRiskParameters().m_transitionTime = hours(transitionTime.hour()) +
    minutes(transitionTime.minute()) + seconds(transitionTime.second()) +
    milliseconds(transitionTime.msec());
  try {
    m_model->Commit();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to save risk parameters: ") + e.what());
  }
}

void RiskWidget::OnCurrencyIndexChanged(int index) {
  auto& currency = m_userProfile->GetCurrencyDatabase().GetEntries()[index];
  m_ui->m_buyingPowerInput->setPrefix(QString::fromStdString(currency.m_sign));
  m_ui->m_buyingPowerInput->setSuffix(QString::fromStdString(
    string{" "} + currency.m_code.GetData()));
  m_ui->m_netLossInput->setPrefix(QString::fromStdString(currency.m_sign));
  m_ui->m_netLossInput->setSuffix(QString::fromStdString(
    string{" "} + currency.m_code.GetData()));
}
