#include "Spire/Blotter/OrderLogPropertiesDialog.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QCheckBox>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_OrderLogPropertiesDialog.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

OrderLogPropertiesDialog::OrderLogPropertiesDialog(
    Ref<UserProfile> userProfile, Ref<BlotterModel> blotterModel,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_OrderLogPropertiesDialog>()),
      m_userProfile(userProfile.Get()),
      m_blotterModel(blotterModel.Get()),
      m_properties(blotterModel->GetOrderLogModel().GetProperties()) {
  m_ui->setupUi(this);
  for(auto status : MakeRange<OrderStatus>()) {
    QCheckBox* checkBox = new QCheckBox(displayText(status));
    m_orderStatusCheckBoxes.insert(make_pair(status, checkBox));
    m_ui->m_orderStatusGroup->layout()->addWidget(checkBox);
    checkBox->setChecked(m_properties.m_orderStatusFilter.Test(status));
  }
  connect(m_ui->m_allOrderStatusButton, &QRadioButton::toggled, this,
    &OrderLogPropertiesDialog::OnAllOrdersClicked);
  connect(m_ui->m_liveOrderStatusButton, &QRadioButton::toggled, this,
    &OrderLogPropertiesDialog::OnLiveOrdersClicked);
  connect(m_ui->m_terminalOrderStatusButton, &QRadioButton::toggled, this,
    &OrderLogPropertiesDialog::OnTerminalOrdersClicked);
  connect(m_ui->m_customOrderStatusButton, &QRadioButton::toggled, this,
    &OrderLogPropertiesDialog::OnCustomOrdersClicked);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &OrderLogPropertiesDialog::OnOkButton);
  connect(m_ui->m_applyAllButton, &QPushButton::clicked, this,
    &OrderLogPropertiesDialog::OnApplyAllButton);
  if(m_properties.m_orderStatusFilterType ==
      OrderLogProperties::OrderStatusFilterType::ALL_ORDERS) {
    m_ui->m_allOrderStatusButton->setChecked(true);
  } else if(m_properties.m_orderStatusFilterType ==
      OrderLogProperties::OrderStatusFilterType::LIVE_ORDERS) {
    m_ui->m_liveOrderStatusButton->setChecked(true);
  } else if(m_properties.m_orderStatusFilterType ==
      OrderLogProperties::OrderStatusFilterType::TERMINAL_ORDERS) {
    m_ui->m_terminalOrderStatusButton->setChecked(true);
  } else if(m_properties.m_orderStatusFilterType ==
      OrderLogProperties::OrderStatusFilterType::CUSTOM) {
    m_ui->m_customOrderStatusButton->setChecked(true);
  }
}

OrderLogPropertiesDialog::~OrderLogPropertiesDialog() {}

void OrderLogPropertiesDialog::OnAllOrdersClicked(bool checked) {
  if(!checked) {
    return;
  }
  m_properties.m_orderStatusFilterType =
    OrderLogProperties::OrderStatusFilterType::ALL_ORDERS;
  for(auto i = m_orderStatusCheckBoxes.begin();
      i != m_orderStatusCheckBoxes.end(); ++i) {
    i->second->setEnabled(false);
    i->second->setChecked(true);
  }
}

void OrderLogPropertiesDialog::OnLiveOrdersClicked(bool checked) {
  if(!checked) {
    return;
  }
  m_properties.m_orderStatusFilterType =
    OrderLogProperties::OrderStatusFilterType::LIVE_ORDERS;
  for(auto i = m_orderStatusCheckBoxes.begin();
      i != m_orderStatusCheckBoxes.end(); ++i) {
    i->second->setEnabled(false);
    i->second->setChecked(!IsTerminal(i->first));
  }
}

void OrderLogPropertiesDialog::OnTerminalOrdersClicked(bool checked) {
  if(!checked) {
    return;
  }
  m_properties.m_orderStatusFilterType =
    OrderLogProperties::OrderStatusFilterType::TERMINAL_ORDERS;
  for(auto i = m_orderStatusCheckBoxes.begin();
      i != m_orderStatusCheckBoxes.end(); ++i) {
    i->second->setEnabled(false);
    i->second->setChecked(IsTerminal(i->first));
  }
}

void OrderLogPropertiesDialog::OnCustomOrdersClicked(bool checked) {
  if(!checked) {
    return;
  }
  m_properties.m_orderStatusFilterType =
    OrderLogProperties::OrderStatusFilterType::CUSTOM;
  for(auto i = m_orderStatusCheckBoxes.begin();
      i != m_orderStatusCheckBoxes.end(); ++i) {
    i->second->setEnabled(true);
  }
}

void OrderLogPropertiesDialog::OnOkButton() {
  m_blotterModel->GetOrderLogModel().SetProperties(m_properties);
  Q_EMIT accept();
}

void OrderLogPropertiesDialog::OnApplyAllButton() {
  BlotterSettings& blotterSettings = m_userProfile->GetBlotterSettings();
  blotterSettings.SetDefaultOrderLogProperties(m_properties);
  for(auto i = blotterSettings.GetAllBlotters().begin();
      i != blotterSettings.GetAllBlotters().end(); ++i) {
    (*i)->GetOrderLogModel().SetProperties(m_properties);
  }
  Q_EMIT accept();
}
