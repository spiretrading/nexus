#include "Spire/KeyBindings/InteractionsWidget.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/MoneySpinBox.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_InteractionsWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

InteractionsWidget::InteractionsWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_InteractionsWidget>()),
      m_properties(InteractionsProperties::GetDefaultProperties()),
      m_quantityModifierIndex(-1),
      m_priceModifierIndex(-1) {
  m_ui->setupUi(this);
  m_ui->m_quantityIncrementModifierComboBox->addItem(QObject::tr("None"));
  m_ui->m_quantityIncrementModifierComboBox->addItem(QObject::tr("Shift"));
  m_ui->m_quantityIncrementModifierComboBox->addItem(QObject::tr("Alt"));
  m_ui->m_quantityIncrementModifierComboBox->addItem(QObject::tr("Ctrl"));
  m_ui->m_priceIncrementModifierComboBox->addItem(QObject::tr("None"));
  m_ui->m_priceIncrementModifierComboBox->addItem(QObject::tr("Shift"));
  m_ui->m_priceIncrementModifierComboBox->addItem(QObject::tr("Alt"));
  m_ui->m_priceIncrementModifierComboBox->addItem(QObject::tr("Ctrl"));
  connect(m_ui->m_regionComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &InteractionsWidget::OnRegionIndexChanged);
  connect(m_ui->m_quantityIncrementModifierComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_priceIncrementModifierComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_activateRegionButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnActivateRegionClicked);
  connect(m_ui->m_resetRegionButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnResetRegionClicked);
}

InteractionsWidget::~InteractionsWidget() {}

void InteractionsWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
  m_properties = m_userProfile->GetInteractionProperties();
  m_ui->m_regionComboBox->clear();
  RegionEntry globalRegion;
  globalRegion.m_region = Region::make_global("Global");
  AddRegion(globalRegion);
  auto countries = m_userProfile->GetCountryDatabase().get_entries();
  for(auto i = countries.begin(); i != countries.end(); ++i) {
    RegionEntry region;
    region.m_region = Region(i->m_name);
    region.m_region += i->m_code;
    AddRegion(region);
  }
  auto venues = m_userProfile->GetVenueDatabase().get_entries();
  for(auto i = venues.begin(); i != venues.end(); ++i) {
    RegionEntry region;
    region.m_region = Region(i->m_display_name);
    region.m_region += i->m_venue;
    AddRegion(region);
  }
  for(auto i = m_properties.begin(); i != m_properties.end(); ++i) {
    if(!std::get<0>(*i).get_securities().empty()) {
      RegionEntry region;
      Security security = *std::get<0>(*i).get_securities().begin();
      region.m_region = Region(displayText(security).toStdString());
      region.m_region += security;
      AddRegion(region);
    }
  }
  Update();
}

void InteractionsWidget::Initialize(Ref<UserProfile> userProfile,
    const Security& security) {
  m_userProfile = userProfile.get();
  m_properties = m_userProfile->GetInteractionProperties();
  m_ui->m_regionComboBox->clear();
  RegionEntry region;
  region.m_region = Region(displayText(security).toStdString());
  region.m_region += security;
  AddRegion(region);
  Update();
}

const RegionMap<InteractionsProperties>& InteractionsWidget::GetProperties() {
  Store();
  return m_properties;
}

void InteractionsWidget::AddRegion(RegionEntry region) {
  region.m_isActive = std::get<0>(*m_properties.find(region.m_region)) ==
    region.m_region;
  m_regions[region.m_region.get_name()] = region;
  m_ui->m_regionComboBox->addItem(QString::fromStdString(
    region.m_region.get_name()));
  int index = m_ui->m_regionComboBox->count() - 1;
  StyleRegion(region);
}

void InteractionsWidget::StyleRegion(const RegionEntry& region) {
  int index = m_ui->m_regionComboBox->findText(
    QString::fromStdString(region.m_region.get_name()));
  if(region.m_isActive) {
    QVariant textColor = m_ui->m_regionComboBox->palette().color(
      QPalette::Normal, QPalette::Text);
    m_ui->m_regionComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    QFont font = qvariant_cast<QFont>(
      m_ui->m_regionComboBox->itemData(index, Qt::FontRole));
    font.setItalic(false);
    m_ui->m_regionComboBox->setItemData(index, font, Qt::FontRole);
  } else {
    QVariant textColor = m_ui->m_regionComboBox->palette().color(
      QPalette::Disabled, QPalette::Text);
    m_ui->m_regionComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    QFont font = qvariant_cast<QFont>(
      m_ui->m_regionComboBox->itemData(index, Qt::FontRole));
    font.setItalic(true);
    m_ui->m_regionComboBox->setItemData(index, font, Qt::FontRole);
  }
}

void InteractionsWidget::Update() {
  m_regionIndex = m_ui->m_regionComboBox->currentText().toStdString();
  m_quantityModifierIndex =
    m_ui->m_quantityIncrementModifierComboBox->currentIndex();
  m_priceModifierIndex = m_ui->m_priceIncrementModifierComboBox->currentIndex();
  const RegionEntry& region = m_regions.at(m_regionIndex);
  const InteractionsProperties& properties = m_properties.get(region.m_region);
  m_ui->m_defaultQuantitySpinBox->setValue(
    static_cast<int>(properties.m_defaultQuantity));
  m_ui->m_quantityIncrementSpinBox->setValue(
    static_cast<int>(properties.m_quantityIncrements[m_quantityModifierIndex]));
  m_ui->m_priceIncrementSpinBox->SetValue(
    properties.m_priceIncrements[m_priceModifierIndex]);
  m_ui->m_cancelOnFillCheckBox->setChecked(properties.m_cancelOnFill);
  m_ui->m_activateRegionButton->setEnabled(region.m_region.is_global());
  if(region.m_isActive) {
    m_ui->m_activateRegionButton->setText(tr("Deactivate"));
  } else {
    m_ui->m_activateRegionButton->setText(tr("Activate"));
  }
  m_ui->m_activateRegionButton->setEnabled(!region.m_region.is_global());
  m_ui->m_resetRegionButton->setEnabled(region.m_isActive);
  m_ui->m_defaultQuantitySpinBox->setEnabled(region.m_isActive);
  m_ui->m_quantityIncrementSpinBox->setEnabled(region.m_isActive);
  m_ui->m_quantityIncrementModifierComboBox->setEnabled(region.m_isActive);
  m_ui->m_priceIncrementSpinBox->setEnabled(region.m_isActive);
  m_ui->m_priceIncrementModifierComboBox->setEnabled(region.m_isActive);
  m_ui->m_cancelOnFillCheckBox->setEnabled(region.m_isActive);
}

void InteractionsWidget::Store() {
  if(m_regionIndex.empty()) {
    return;
  }
  InteractionsProperties& properties = m_properties.get(
    m_regions.at(m_regionIndex).m_region);
  properties.m_defaultQuantity = m_ui->m_defaultQuantitySpinBox->value();
  if(m_quantityModifierIndex != -1) {
    properties.m_quantityIncrements[m_quantityModifierIndex] =
      m_ui->m_quantityIncrementSpinBox->value();
  }
  if(m_priceModifierIndex != -1) {
    properties.m_priceIncrements[m_priceModifierIndex] =
      m_ui->m_priceIncrementSpinBox->GetValue();
  }
  properties.m_cancelOnFill = m_ui->m_cancelOnFillCheckBox->isChecked();
}

void InteractionsWidget::OnRegionIndexChanged(int index) {
  Store();
  Update();
}

void InteractionsWidget::OnKeyboardModifierIndexChanged(int index) {
  Store();
  Update();
}

void InteractionsWidget::OnActivateRegionClicked() {
  if(m_regionIndex.empty()) {
    return;
  }
  RegionEntry& region = m_regions.at(m_regionIndex);
  if(!region.m_isActive) {
    region.m_isActive = true;
    m_properties.set(region.m_region,
      InteractionsProperties::GetDefaultProperties());
    Store();
  } else {
    region.m_isActive = false;
    m_properties.erase(region.m_region);
  }
  StyleRegion(region);
  Update();
}

void InteractionsWidget::OnResetRegionClicked() {
  const RegionEntry& region = m_regions.at(m_regionIndex);
  m_properties.get(region.m_region) =
    InteractionsProperties::GetDefaultProperties();
  Update();
}
