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
  connect(m_ui->m_scopeComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &InteractionsWidget::OnScopeIndexChanged);
  connect(m_ui->m_quantityIncrementModifierComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_priceIncrementModifierComboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_activateScopeButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnActivateScopeClicked);
  connect(m_ui->m_resetScopeButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnResetScopeClicked);
}

InteractionsWidget::~InteractionsWidget() {}

void InteractionsWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
  m_properties = m_userProfile->GetInteractionProperties();
  m_ui->m_scopeComboBox->clear();
  ScopeEntry globalScope;
  globalScope.m_scope = Scope::make_global("Global");
  AddScope(globalScope);
  auto countries = m_userProfile->GetCountryDatabase().get_entries();
  for(auto i = countries.begin(); i != countries.end(); ++i) {
    ScopeEntry scope;
    scope.m_scope = Scope(i->m_name);
    scope.m_scope += i->m_code;
    AddScope(scope);
  }
  auto venues = m_userProfile->GetVenueDatabase().get_entries();
  for(auto i = venues.begin(); i != venues.end(); ++i) {
    ScopeEntry scope;
    scope.m_scope = Scope(i->m_display_name);
    scope.m_scope += i->m_venue;
    AddScope(scope);
  }
  for(auto i = m_properties.begin(); i != m_properties.end(); ++i) {
    if(!std::get<0>(*i).get_tickers().empty()) {
      ScopeEntry scope;
      Ticker ticker = *std::get<0>(*i).get_tickers().begin();
      scope.m_scope = Scope(displayText(ticker).toStdString());
      scope.m_scope += ticker;
      AddScope(scope);
    }
  }
  Update();
}

void InteractionsWidget::Initialize(Ref<UserProfile> userProfile,
    const Ticker& ticker) {
  m_userProfile = userProfile.get();
  m_properties = m_userProfile->GetInteractionProperties();
  m_ui->m_scopeComboBox->clear();
  ScopeEntry scope;
  scope.m_scope = Scope(displayText(ticker).toStdString());
  scope.m_scope += ticker;
  AddScope(scope);
  Update();
}

const ScopeTable<InteractionsProperties>& InteractionsWidget::GetProperties() {
  Store();
  return m_properties;
}

void InteractionsWidget::AddScope(ScopeEntry scope) {
  scope.m_isActive = std::get<0>(*m_properties.find(scope.m_scope)) ==
    scope.m_scope;
  m_scopes[scope.m_scope.get_name()] = scope;
  m_ui->m_scopeComboBox->addItem(
    QString::fromStdString(scope.m_scope.get_name()));
  int index = m_ui->m_scopeComboBox->count() - 1;
  StyleScope(scope);
}

void InteractionsWidget::StyleScope(const ScopeEntry& scope) {
  int index = m_ui->m_scopeComboBox->findText(
    QString::fromStdString(scope.m_scope.get_name()));
  if(scope.m_isActive) {
    QVariant textColor = m_ui->m_scopeComboBox->palette().color(
      QPalette::Normal, QPalette::Text);
    m_ui->m_scopeComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    QFont font = qvariant_cast<QFont>(
      m_ui->m_scopeComboBox->itemData(index, Qt::FontRole));
    font.setItalic(false);
    m_ui->m_scopeComboBox->setItemData(index, font, Qt::FontRole);
  } else {
    QVariant textColor = m_ui->m_scopeComboBox->palette().color(
      QPalette::Disabled, QPalette::Text);
    m_ui->m_scopeComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    QFont font = qvariant_cast<QFont>(
      m_ui->m_scopeComboBox->itemData(index, Qt::FontRole));
    font.setItalic(true);
    m_ui->m_scopeComboBox->setItemData(index, font, Qt::FontRole);
  }
}

void InteractionsWidget::Update() {
  m_scopeIndex = m_ui->m_scopeComboBox->currentText().toStdString();
  m_quantityModifierIndex =
    m_ui->m_quantityIncrementModifierComboBox->currentIndex();
  m_priceModifierIndex = m_ui->m_priceIncrementModifierComboBox->currentIndex();
  const ScopeEntry& scope = m_scopes.at(m_scopeIndex);
  const InteractionsProperties& properties = m_properties.get(scope.m_scope);
  m_ui->m_defaultQuantitySpinBox->setValue(
    static_cast<int>(properties.m_defaultQuantity));
  m_ui->m_quantityIncrementSpinBox->setValue(
    static_cast<int>(properties.m_quantityIncrements[m_quantityModifierIndex]));
  m_ui->m_priceIncrementSpinBox->SetValue(
    properties.m_priceIncrements[m_priceModifierIndex]);
  m_ui->m_cancelOnFillCheckBox->setChecked(properties.m_cancelOnFill);
  m_ui->m_activateScopeButton->setEnabled(scope.m_scope.is_global());
  if(scope.m_isActive) {
    m_ui->m_activateScopeButton->setText(tr("Deactivate"));
  } else {
    m_ui->m_activateScopeButton->setText(tr("Activate"));
  }
  m_ui->m_activateScopeButton->setEnabled(!scope.m_scope.is_global());
  m_ui->m_resetScopeButton->setEnabled(scope.m_isActive);
  m_ui->m_defaultQuantitySpinBox->setEnabled(scope.m_isActive);
  m_ui->m_quantityIncrementSpinBox->setEnabled(scope.m_isActive);
  m_ui->m_quantityIncrementModifierComboBox->setEnabled(scope.m_isActive);
  m_ui->m_priceIncrementSpinBox->setEnabled(scope.m_isActive);
  m_ui->m_priceIncrementModifierComboBox->setEnabled(scope.m_isActive);
  m_ui->m_cancelOnFillCheckBox->setEnabled(scope.m_isActive);
}

void InteractionsWidget::Store() {
  if(m_scopeIndex.empty()) {
    return;
  }
  InteractionsProperties& properties =
    m_properties.get(m_scopes.at(m_scopeIndex).m_scope);
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

void InteractionsWidget::OnScopeIndexChanged(int index) {
  Store();
  Update();
}

void InteractionsWidget::OnKeyboardModifierIndexChanged(int index) {
  Store();
  Update();
}

void InteractionsWidget::OnActivateScopeClicked() {
  if(m_scopeIndex.empty()) {
    return;
  }
  ScopeEntry& scope = m_scopes.at(m_scopeIndex);
  if(!scope.m_isActive) {
    scope.m_isActive = true;
    m_properties.set(
      scope.m_scope, InteractionsProperties::GetDefaultProperties());
    Store();
  } else {
    scope.m_isActive = false;
    m_properties.erase(scope.m_scope);
  }
  StyleScope(scope);
  Update();
}

void InteractionsWidget::OnResetScopeClicked() {
  const ScopeEntry& scope = m_scopes.at(m_scopeIndex);
  m_properties.get(scope.m_scope) =
    InteractionsProperties::GetDefaultProperties();
  Update();
}
