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
  connect(m_ui->m_scopeComboBox,
    qOverload<int>(&QComboBox::currentIndexChanged), this,
    &InteractionsWidget::OnScopeIndexChanged);
  connect(m_ui->m_quantityIncrementModifierComboBox,
    qOverload<int>(&QComboBox::currentIndexChanged), this,
    &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_priceIncrementModifierComboBox,
    qOverload<int>(&QComboBox::currentIndexChanged), this,
    &InteractionsWidget::OnKeyboardModifierIndexChanged);
  connect(m_ui->m_activateScopeButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnActivateScopeClicked);
  connect(m_ui->m_resetScopeButton, &QPushButton::clicked, this,
    &InteractionsWidget::OnResetScopeClicked);
}

InteractionsWidget::~InteractionsWidget() = default;

void InteractionsWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
  m_ui->m_scopeComboBox->clear();
  auto globalScope = ScopeEntry();
  globalScope.m_scope = Scope::make_global("Global");
  Add(globalScope);
  for(auto& country : DEFAULT_COUNTRIES.get_entries()) {
    auto scope = ScopeEntry();
    scope.m_scope = Scope(country.m_name);
    scope.m_scope += country.m_code;
    Add(scope);
  }
  for(auto& venue : DEFAULT_VENUES.get_entries()) {
    auto scope = ScopeEntry();
    scope.m_scope = Scope(venue.m_display_name);
    scope.m_scope += venue.m_venue;
    Add(scope);
  }
  auto scopes =
    m_userProfile->GetKeyBindings()->make_interactions_key_bindings_scopes();
  for(auto& scopeKey : scopes) {
    if(scopeKey.get_tickers().empty()) {
      continue;
    }
    auto scope = ScopeEntry();
    auto ticker = *scopeKey.get_tickers().begin();
    scope.m_scope = Scope(to_string(ticker));
    scope.m_scope += ticker;
    Add(scope);
  }
  Update();
}

void InteractionsWidget::Initialize(
    Ref<UserProfile> userProfile, const Ticker& ticker) {
  m_userProfile = userProfile.get();
  m_ui->m_scopeComboBox->clear();
  auto scope = ScopeEntry();
  scope.m_scope = Scope(to_string(ticker));
  scope.m_scope += ticker;
  Add(scope);
  Update();
}

void InteractionsWidget::Store() {
  if(m_scopeIndex.empty()) {
    return;
  }
  auto interactions =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      m_scopes.at(m_scopeIndex).m_scope);
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

void InteractionsWidget::Add(ScopeEntry scope) {
  auto scopes =
    m_userProfile->GetKeyBindings()->make_interactions_key_bindings_scopes();
  scope.m_isActive =
    std::find(scopes.begin(), scopes.end(), scope.m_scope) != scopes.end();
  m_scopes[scope.m_scope.get_name()] = scope;
  m_ui->m_scopeComboBox->addItem(
    QString::fromStdString(scope.m_scope.get_name()));
  Style(scope);
}

void InteractionsWidget::Style(const ScopeEntry& scope) {
  auto index = m_ui->m_scopeComboBox->findText(
    QString::fromStdString(scope.m_scope.get_name()));
  if(scope.m_isActive) {
    auto textColor =
      m_ui->m_scopeComboBox->palette().color(QPalette::Normal, QPalette::Text);
    m_ui->m_scopeComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    auto font = qvariant_cast<QFont>(
      m_ui->m_scopeComboBox->itemData(index, Qt::FontRole));
    font.setItalic(false);
    m_ui->m_scopeComboBox->setItemData(index, font, Qt::FontRole);
  } else {
    auto textColor = m_ui->m_scopeComboBox->palette().color(
      QPalette::Disabled, QPalette::Text);
    m_ui->m_scopeComboBox->setItemData(index, textColor, Qt::ForegroundRole);
    auto font = qvariant_cast<QFont>(
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
  auto& scope = m_scopes.at(m_scopeIndex);
  auto interactions =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      scope.m_scope);
  m_ui->m_defaultQuantitySpinBox->setValue(
    static_cast<int>(interactions->get_default_quantity()->get()));
  m_ui->m_quantityIncrementSpinBox->setValue(
    static_cast<int>(interactions->get_quantity_increment(
      make_modifier(m_quantityModifierIndex))->get()));
  m_ui->m_priceIncrementSpinBox->SetValue(interactions->get_price_increment(
    make_modifier(m_priceModifierIndex))->get());
  m_ui->m_cancelOnFillCheckBox->setChecked(
    interactions->is_cancel_on_fill()->get());
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
  auto& scope = m_scopes.at(m_scopeIndex);
  if(!scope.m_isActive) {
    scope.m_isActive = true;
    Store();
  } else {
    scope.m_isActive = false;
    auto interactions =
      m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
        m_scopes.at(m_scopeIndex).m_scope);
    interactions->reset();
  }
  Style(scope);
  Update();
}

void InteractionsWidget::OnResetScopeClicked() {
  auto& scope = m_scopes.at(m_scopeIndex);
  auto interactions =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      m_scopes.at(m_scopeIndex).m_scope);
  interactions->reset();
  Update();
}
