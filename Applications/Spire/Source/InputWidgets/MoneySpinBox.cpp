#include "Spire/InputWidgets/MoneySpinBox.hpp"
#include <QApplication>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QVBoxLayout>

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

MoneySpinBox::MoneySpinBox(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_userProfile(nullptr),
      m_isReadOnly(false) {
  auto layout = new QVBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_spinBox = new QDoubleSpinBox{this};
  m_spinBox->setMaximum(1000000000);
  m_spinBox->setMinimum(-1000000000);
  m_spinBox->setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  m_spinBox->setKeyboardTracking(true);
  m_spinBox->setAccelerated(false);
  m_spinBox->setSingleStep(0.01);
  m_spinBox->setDecimals(6);
  m_spinBox->installEventFilter(this);
  layout->addWidget(m_spinBox);
  connect(m_spinBox, qOverload<const QString&>(
    &QDoubleSpinBox::textChanged), this, &MoneySpinBox::OnValueChanged);
}

MoneySpinBox::MoneySpinBox(Ref<UserProfile> userProfile, QWidget* parent,
    Qt::WindowFlags flags)
    : MoneySpinBox{parent, flags} {
  Initialize(Ref(userProfile));
}

void MoneySpinBox::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
  AdjustIncrement(Qt::NoModifier);
}

const optional<Security>& MoneySpinBox::GetLinkedSecurity() const {
  return m_security;
}

void MoneySpinBox::SetLinkedSecurity(const optional<Security>& security) {
  m_security = security;
  AdjustIncrement(Qt::NoModifier);
}

void MoneySpinBox::SetDecimals(int decimalCount) {
  m_spinBox->setDecimals(decimalCount);
}

int MoneySpinBox::GetDecimals() const {
  return m_spinBox->decimals();
}

Money MoneySpinBox::GetValue() const {
  return parse_money(m_spinBox->cleanText().toStdString());
}

void MoneySpinBox::SetValue(Money value) {
  m_spinBox->setValue(
    static_cast<Quantity>(value).get_representation() / Quantity::MULTIPLIER);
  m_valueUpdatedSignal(value);
}

void MoneySpinBox::SetReadOnly(bool value) {
  m_isReadOnly = value;
  m_spinBox->setReadOnly(value);
}

connection MoneySpinBox::ConnectValueUpdatedSignal(
    const ValueUpdatedSignal::slot_type& slot) const {
  return m_valueUpdatedSignal.connect(slot);
}

bool MoneySpinBox::eventFilter(QObject* receiver, QEvent* event) {
  if(receiver == m_spinBox) {
    if(event->type() == QEvent::KeyPress) {
      auto keyEvent = static_cast<QKeyEvent*>(event);
      AdjustIncrement(to_modifier(keyEvent->modifiers()));
    } else if(event->type() == QEvent::KeyRelease) {
      auto keyEvent = static_cast<QKeyEvent*>(event);
      if(keyEvent->modifiers() == Qt::SHIFT ||
          keyEvent->modifiers() == Qt::ALT ||
          keyEvent->modifiers() == Qt::CTRL) {
        AdjustIncrement(Qt::NoModifier);
      }
    }
  }
  return QWidget::eventFilter(receiver, event);
}

void MoneySpinBox::AdjustIncrement(Qt::KeyboardModifier modifier) {
  if(!m_userProfile || !m_security) {
    return;
  }
  auto priceIncrement =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      *m_security)->get_price_increment(modifier)->get();
  auto increment = static_cast<Quantity>(priceIncrement) / Quantity::MULTIPLIER;
  if(increment != m_spinBox->singleStep()) {
    m_spinBox->setSingleStep(static_cast<double>(increment));
  }
}

void MoneySpinBox::OnValueChanged(const QString& value) {
  m_valueUpdatedSignal(GetValue());
}
