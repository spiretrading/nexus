#include "Spire/Blotter/ProfitAndLossHeader.hpp"
#include <QLayout>
#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "ui_ProfitAndLossHeader.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

ProfitAndLossHeader::ProfitAndLossHeader(Ref<ProfitAndLossEntryModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui_ProfitAndLossHeader>()),
      m_model(model.get()) {
  m_ui->setupUi(this);
  m_ui->m_currencyCode->setText(QString::fromStdString(
    m_model->GetCurrency().m_code.get_data()));
  m_ui->m_currencySymbol->setText(QString::fromStdString(
    m_model->GetCurrency().m_sign));
  m_profitAndLossConnection = m_model->ConnectProfitAndLossSignal(
    std::bind(&ProfitAndLossHeader::OnProfitAndLossChanged, this,
    std::placeholders::_1));
  m_volumeConnection = m_model->ConnectVolumeSignal(
    std::bind(&ProfitAndLossHeader::OnVolumeChanged, this,
    std::placeholders::_1));
  OnProfitAndLossChanged(Money::ZERO);
  OnVolumeChanged(0);
}

ProfitAndLossHeader::~ProfitAndLossHeader() {}

void ProfitAndLossHeader::OnProfitAndLossChanged(Money profitAndLoss) {
  m_ui->m_profitLossValue->setText(QString::fromStdString(
    lexical_cast<string>(profitAndLoss)));
  if(profitAndLoss > Money::ZERO) {
    m_ui->m_profitLossValue->setStyleSheet("\
      QLabel {\
        color: rgb(0, 119, 53);\
      };");
  } else if(profitAndLoss == Money::ZERO) {
    m_ui->m_profitLossValue->setStyleSheet("\
      QLabel {\
        color: rgb(0, 0, 0);\
      };");
  } else {
    m_ui->m_profitLossValue->setStyleSheet("\
      QLabel {\
        color: rgb(192, 0, 0);\
      };");
  }
}

void ProfitAndLossHeader::OnVolumeChanged(Quantity volume) {
  m_ui->m_volumeValue->setText(QString::number(static_cast<int>(volume)));
}
