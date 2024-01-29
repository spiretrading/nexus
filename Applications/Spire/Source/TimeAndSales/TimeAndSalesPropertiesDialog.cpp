#include "Spire/TimeAndSales/TimeAndSalesPropertiesDialog.hpp"
#include <QColorDialog>
#include <QFontDialog>
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "ui_TimeAndSalesPropertiesDialog.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  QString GetButtonStyle(const QColor& color) {
    QString style = "QPushButton {background-color: " + color.name() + ";}";
    return style;
  }
}

TimeAndSalesPropertiesDialog::TimeAndSalesPropertiesDialog(
    Ref<UserProfile> userProfile, const TimeAndSalesProperties& properties,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_TimeAndSalesPropertiesDialog>()),
      m_userProfile(userProfile.Get()),
      m_properties(properties) {
  m_ui->setupUi(this);
  m_fontChangedConnection = m_ui->m_fontSelector->ConnectFontChangedSignal(
    std::bind(&TimeAndSalesPropertiesDialog::OnFontChanged, this,
    std::placeholders::_1, std::placeholders::_2));
  connect(m_ui->m_priceRangeList, &QListWidget::currentItemChanged, this,
    &TimeAndSalesPropertiesDialog::OnPriceRangeItemChanged);
  connect(m_ui->m_foregroundButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnForegroundClicked);
  connect(m_ui->m_backgroundButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnBackgroundClicked);
  connect(m_ui->m_columnButtonGroup, static_cast<void (QButtonGroup::*)(
    QAbstractButton*)>(&QButtonGroup::buttonClicked), this,
    &TimeAndSalesPropertiesDialog::OnColumnCheckBoxClicked);
  connect(m_ui->m_showGridLineCheckBox, &QCheckBox::stateChanged, this,
    &TimeAndSalesPropertiesDialog::OnShowGridClicked);
  connect(m_ui->m_horizontalScrollingCheckBox, &QCheckBox::stateChanged, this,
    &TimeAndSalesPropertiesDialog::OnHorizontalScrollBoxClicked);
  connect(m_ui->m_verticalScrollingCheckBox, &QCheckBox::stateChanged, this,
    &TimeAndSalesPropertiesDialog::OnVerticalScrollBoxClicked);
  connect(m_ui->m_loadDefaultButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnLoadDefault);
  connect(m_ui->m_saveAsDefaultButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnSaveAsDefault);
  connect(m_ui->m_resetDefaultButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnResetDefault);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::accept);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::reject);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnApply);
  connect(m_ui->m_applyToAllButton, &QPushButton::clicked, this,
    &TimeAndSalesPropertiesDialog::OnApplyToAll);
  Redisplay();
}

TimeAndSalesPropertiesDialog::~TimeAndSalesPropertiesDialog() {}

const TimeAndSalesProperties&
    TimeAndSalesPropertiesDialog::GetProperties() const {
  return m_properties;
}

void TimeAndSalesPropertiesDialog::Redisplay() {
  m_ui->m_priceRangeList->clear();
  m_priceRangeItems[TimeAndSalesModel::UNKNOWN] = new QListWidgetItem(
    tr("Bid/Ask Unknown"));
  m_priceRangeItems[TimeAndSalesModel::ABOVE_ASK] = new QListWidgetItem(
    tr("Trade Above Ask"));
  m_priceRangeItems[TimeAndSalesModel::AT_ASK] = new QListWidgetItem(
    tr("Trade At Ask"));
  m_priceRangeItems[TimeAndSalesModel::INSIDE] = new QListWidgetItem(
    tr("Trade Inside"));
  m_priceRangeItems[TimeAndSalesModel::AT_BID] = new QListWidgetItem(
    tr("Trade At Bid"));
  m_priceRangeItems[TimeAndSalesModel::BELOW_BID] = new QListWidgetItem(
    tr("Trade Below Bid"));
  for(int i = 0; i < TimeAndSalesProperties::PRICE_RANGE_COUNT; ++i) {
    m_priceRangeItems[i]->setBackground(
      m_properties.GetPriceRangeBackgroundColor()[i]);
    m_priceRangeItems[i]->setForeground(
      m_properties.GetPriceRangeForegroundColor()[i]);
    m_ui->m_priceRangeList->addItem(m_priceRangeItems[i]);
  }
  m_ui->m_priceRangeList->setCurrentRow(0);
  m_ui->m_timeColumnCheckBox->setChecked(
    m_properties.GetVisibleColumns()[TimeAndSalesModel::TIME_COLUMN]);
  m_ui->m_priceColumnCheckBox->setChecked(
    m_properties.GetVisibleColumns()[TimeAndSalesModel::PRICE_COLUMN]);
  m_ui->m_sizeColumnCheckBox->setChecked(
    m_properties.GetVisibleColumns()[TimeAndSalesModel::SIZE_COLUMN]);
  m_ui->m_marketColumnCheckBox->setChecked(
    m_properties.GetVisibleColumns()[TimeAndSalesModel::MARKET_COLUMN]);
  m_ui->m_conditionColumnCheckBox->setChecked(
    m_properties.GetVisibleColumns()[TimeAndSalesModel::CONDITION_COLUMN]);
  m_ui->m_showGridLineCheckBox->setChecked(m_properties.GetShowGridLines());
  m_ui->m_horizontalScrollingCheckBox->setChecked(
    m_properties.IsHorizontalScrollBarVisible());
  m_ui->m_verticalScrollingCheckBox->setChecked(
    m_properties.IsVerticalScrollBarVisible());
  m_ui->m_priceRangeList->setFont(m_properties.GetFont());
  m_ui->m_backgroundButton->setStyleSheet(GetButtonStyle(
    m_properties.GetPriceRangeBackgroundColor()[
    m_ui->m_priceRangeList->currentRow()]));
  m_ui->m_foregroundButton->setStyleSheet(GetButtonStyle(
    m_properties.GetPriceRangeForegroundColor()[
    m_ui->m_priceRangeList->currentRow()]));
  m_ui->m_fontSelector->SetFont(m_properties.GetFont());
}

void TimeAndSalesPropertiesDialog::OnFontChanged(const QFont& oldFont,
    const QFont& newFont) {
  m_properties.SetFont(newFont);
  m_ui->m_priceRangeList->setFont(m_properties.GetFont());
}

void TimeAndSalesPropertiesDialog::OnPriceRangeItemChanged(
    QListWidgetItem* current, QListWidgetItem* previous) {
  if(current == nullptr) {
    return;
  }
  m_ui->m_backgroundButton->setStyleSheet(GetButtonStyle(
    m_properties.GetPriceRangeBackgroundColor()[
    m_ui->m_priceRangeList->currentRow()]));
  m_ui->m_foregroundButton->setStyleSheet(GetButtonStyle(
    m_properties.GetPriceRangeForegroundColor()[
    m_ui->m_priceRangeList->currentRow()]));
}

void TimeAndSalesPropertiesDialog::OnForegroundClicked() {
  QColor color = QColorDialog::getColor(
    m_properties.GetPriceRangeForegroundColor()[
    m_ui->m_priceRangeList->currentRow()], this);
  if(!color.isValid()) {
    return;
  }
  m_ui->m_priceRangeList->currentItem()->setForeground(color);
  m_properties.GetPriceRangeForegroundColor()[
    m_ui->m_priceRangeList->currentRow()] = color;
  m_ui->m_foregroundButton->setStyleSheet(GetButtonStyle(color));
}

void TimeAndSalesPropertiesDialog::OnBackgroundClicked() {
  QColor color = QColorDialog::getColor(
    m_properties.GetPriceRangeBackgroundColor()[
    m_ui->m_priceRangeList->currentRow()], this);
  if(!color.isValid()) {
    return;
  }
  m_ui->m_priceRangeList->currentItem()->setBackground(color);
  m_properties.GetPriceRangeBackgroundColor()[
    m_ui->m_priceRangeList->currentRow()] = color;
  m_ui->m_backgroundButton->setStyleSheet(GetButtonStyle(color));
}

void TimeAndSalesPropertiesDialog::OnColumnCheckBoxClicked(
    QAbstractButton* button) {
  int index;
  if(button == m_ui->m_timeColumnCheckBox) {
    index = TimeAndSalesModel::TIME_COLUMN;
  } else if(button == m_ui->m_priceColumnCheckBox) {
    index = TimeAndSalesModel::PRICE_COLUMN;
  } else if(button == m_ui->m_sizeColumnCheckBox) {
    index = TimeAndSalesModel::SIZE_COLUMN;
  } else if(button == m_ui->m_marketColumnCheckBox) {
    index = TimeAndSalesModel::MARKET_COLUMN;
  } else {
    assert(button == m_ui->m_conditionColumnCheckBox);
    index = TimeAndSalesModel::CONDITION_COLUMN;
  }
  QCheckBox* checkBox = static_cast<QCheckBox*>(button);
  m_properties.GetVisibleColumns()[index] = checkBox->isChecked();
}

void TimeAndSalesPropertiesDialog::OnShowGridClicked(int state) {
  if(state == Qt::Unchecked) {
    m_properties.SetShowGridLines(false);
  } else if(state == Qt::Checked) {
    m_properties.SetShowGridLines(true);
  }
}

void TimeAndSalesPropertiesDialog::OnHorizontalScrollBoxClicked(int state) {
  if(state == Qt::Unchecked) {
    m_properties.SetHorizontalScrollBarVisible(false);
  } else if(state == Qt::Checked) {
    m_properties.SetHorizontalScrollBarVisible(true);
  }
}

void TimeAndSalesPropertiesDialog::OnVerticalScrollBoxClicked(int state) {
  if(state == Qt::Unchecked) {
    m_properties.SetVerticalScrollBarVisible(false);
  } else if(state == Qt::Checked) {
    m_properties.SetVerticalScrollBarVisible(true);
  }
}

void TimeAndSalesPropertiesDialog::OnLoadDefault() {
  m_properties = m_userProfile->GetDefaultTimeAndSalesProperties();
  Redisplay();
}

void TimeAndSalesPropertiesDialog::OnSaveAsDefault() {
  m_userProfile->SetDefaultTimeAndSalesProperties(m_properties);
}

void TimeAndSalesPropertiesDialog::OnResetDefault() {
  m_userProfile->SetDefaultTimeAndSalesProperties(
    TimeAndSalesProperties::GetDefault());
}

void TimeAndSalesPropertiesDialog::OnApply() {
  TimeAndSalesWindow* window = dynamic_cast<TimeAndSalesWindow*>(parent());
  if(window == nullptr) {
    return;
  }
  window->SetProperties(m_properties);
}

void TimeAndSalesPropertiesDialog::OnApplyToAll() {
  QWidgetList widgets = QApplication::topLevelWidgets();
  for(auto i = widgets.begin(); i != widgets.end(); ++i) {
    TimeAndSalesWindow* window = dynamic_cast<TimeAndSalesWindow*>(*i);
    if(window != nullptr) {
      window->SetProperties(m_properties);
    }
  }
}
