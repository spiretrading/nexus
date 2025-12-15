#include "Spire/KeyBindings/InteractionsWidget.hpp"
#include <stdexcept>
#include <Beam/Utilities/ToString.hpp>
#include "Spire/LegacyUI/MoneySpinBox.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_InteractionsWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  Qt::KeyboardModifier make_modifier(int index) {
    if(index == 0) {
      return Qt::NoModifier;
    } else if(index == 1) {
      return Qt::ShiftModifier;
    } else if(index == 2) {
      return Qt::AltModifier;
    } else if(index == 3) {
      return Qt::ControlModifier;
    }
    throw std::out_of_range("Key modifier not found.");
  }
}

InteractionsWidget::InteractionsWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_InteractionsWidget>()),
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
    qOverload<int>(&QComboBox::currentIndexChanged), this,
    &InteractionsWidget::OnRegionIndexChanged);
  connect(m_ui->m_quantityIncrementModifierComboBox,
    qOverload<int>(&QComboBox::currentIndexChanged), this,
    &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_priceIncrementModifierComboBox,
    qOverload<int>(&QComboBox::currentIndexChanged), this,
    &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_activateRegionButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnActivateRegionClicked);
  connect(m_ui->m_resetRegionButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnResetRegionClicked);
}

InteractionsWidget::~InteractionsWidget() = default;

void InteractionsWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
  m_ui->m_regionComboBox->clear();
  auto globalRegion = RegionEntry();
  globalRegion.m_region = Region::make_global("Global");
  Add(globalRegion);
  for(auto& country : DEFAULT_COUNTRIES.get_entries()) {
    auto region = RegionEntry();
    region.m_region = Region(country.m_name);
    region.m_region += country.m_code;
    Add(region);
  }
  for(auto& venue : DEFAULT_VENUES.get_entries()) {
    auto region = RegionEntry();
    region.m_region = Region(venue.m_display_name);
    region.m_region += venue.m_venue;
    Add(region);
  }
  auto regions =
    m_userProfile->GetKeyBindings()->make_interactions_key_bindings_regions();
  for(auto& regionKey : regions) {
    if(regionKey.get_securities().empty()) {
      continue;
    }
    auto region = RegionEntry();
    auto security = *regionKey.get_securities().begin();
    region.m_region = Region(to_string(security));
    region.m_region += security;
    Add(region);
  }
  Update();
}

void InteractionsWidget::Initialize(
    Ref<UserProfile> userProfile, const Security& security) {
  m_userProfile = userProfile.get();
  m_ui->m_regionComboBox->clear();
  auto region = RegionEntry();
  region.m_region = Region(to_string(security));
  region.m_region += security;
  Add(region);
  Update();
}

void InteractionsWidget::Add(RegionEntry region) {
  auto regions =
    m_userProfile->GetKeyBindings()->make_interactions_key_bindings_regions();
  region.m_isActive =
    std::find(regions.begin(), regions.end(), region.m_region) != regions.end();
  m_regions[region.m_region.get_name()] = region;
  m_ui->m_regionComboBox->addItem(
    QString::fromStdString(region.m_region.get_name()));
  Style(region);
}

void InteractionsWidget::Style(const RegionEntry& region) {
  auto index = m_ui->m_regionComboBox->findText(
    QString::fromStdString(region.m_region.get_name()));
  if(region.m_isActive) {
    auto textColor =
      m_ui->m_regionComboBox->palette().color(QPalette::Normal, QPalette::Text);
    m_ui->m_regionComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    auto font = qvariant_cast<QFont>(
      m_ui->m_regionComboBox->itemData(index, Qt::FontRole));
    font.setItalic(false);
    m_ui->m_regionComboBox->setItemData(index, font, Qt::FontRole);
  } else {
    auto textColor = m_ui->m_regionComboBox->palette().color(
      QPalette::Disabled, QPalette::Text);
    m_ui->m_regionComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    auto font = qvariant_cast<QFont>(
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
  auto& region = m_regions.at(m_regionIndex);
  auto interactions =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      region.m_region);
  m_ui->m_defaultQuantitySpinBox->setValue(
    static_cast<int>(interactions->get_default_quantity()->get()));
  m_ui->m_quantityIncrementSpinBox->setValue(
    static_cast<int>(interactions->get_quantity_increment(
      make_modifier(m_quantityModifierIndex))->get()));
  m_ui->m_priceIncrementSpinBox->SetValue(interactions->get_price_increment(
    make_modifier(m_priceModifierIndex))->get());
  m_ui->m_cancelOnFillCheckBox->setChecked(
    interactions->is_cancel_on_fill()->get());
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
  auto interactions =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      m_regions.at(m_regionIndex).m_region);
  interactions->get_default_quantity()->set(
    m_ui->m_defaultQuantitySpinBox->value());
  if(m_quantityModifierIndex != -1) {
    interactions->get_quantity_increment(
      make_modifier(m_quantityModifierIndex))->set(
        m_ui->m_quantityIncrementSpinBox->value());
  }
  if(m_priceModifierIndex != -1) {
    interactions->get_price_increment(make_modifier(m_priceModifierIndex))->set(
      m_ui->m_priceIncrementSpinBox->GetValue());
  }
  interactions->is_cancel_on_fill()->set(
    m_ui->m_cancelOnFillCheckBox->isChecked());
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
  auto& region = m_regions.at(m_regionIndex);
  if(!region.m_isActive) {
    region.m_isActive = true;
    Store();
  } else {
    region.m_isActive = false;
    auto interactions =
      m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
        m_regions.at(m_regionIndex).m_region);
    interactions->reset();
  }
  Style(region);
  Update();
}

void InteractionsWidget::OnResetRegionClicked() {
  auto& region = m_regions.at(m_regionIndex);
  auto interactions =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      m_regions.at(m_regionIndex).m_region);
  interactions->reset();
  Update();
}
