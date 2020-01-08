#include "Spire/InputWidgets/MoneySpinBox.hpp"
#include <QApplication>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace std;

MoneySpinBox::MoneySpinBox(QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_userProfile{nullptr},
      m_isReadOnly{false} {
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
  connect(m_spinBox,
    static_cast<void (QDoubleSpinBox::*)(const QString&)>(
    &QDoubleSpinBox::textChanged), this, &MoneySpinBox::OnValueChanged);
}

MoneySpinBox::MoneySpinBox(Ref<UserProfile> userProfile, QWidget* parent,
    Qt::WindowFlags flags)
    : MoneySpinBox{parent, flags} {
  Initialize(Ref(userProfile));
}

void MoneySpinBox::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.Get();
  AdjustIncrement(KeyModifiers::PLAIN);
}

const boost::optional<Security>& MoneySpinBox::GetLinkedSecurity() const {
  return m_security;
}

void MoneySpinBox::SetLinkedSecurity(
    const boost::optional<Security>& security) {
  m_security = security;
  AdjustIncrement(KeyModifiers::PLAIN);
}

void MoneySpinBox::SetDecimals(int decimalCount) {
  m_spinBox->setDecimals(decimalCount);
}

int MoneySpinBox::GetDecimals() const {
  return m_spinBox->decimals();
}

Money MoneySpinBox::GetValue() const {
  auto value = Money::FromValue(m_spinBox->cleanText().toStdString());
  assert(value.is_initialized());
  return *value;
}

void MoneySpinBox::SetValue(Money value) {
  m_spinBox->setValue(static_cast<Quantity>(value).GetRepresentation() /
    Quantity::MULTIPLIER);
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
      AdjustIncrement(KeyModifiersFromEvent(*keyEvent));
    } else if(event->type() == QEvent::KeyRelease) {
      auto keyEvent = static_cast<QKeyEvent*>(event);
      if(keyEvent->modifiers() == Qt::SHIFT ||
          keyEvent->modifiers() == Qt::ALT ||
          keyEvent->modifiers() == Qt::CTRL) {
        AdjustIncrement(KeyModifiers::PLAIN);
      }
    }
  }
  return QWidget::eventFilter(receiver, event);
}

void MoneySpinBox::AdjustIncrement(KeyModifiers modifier) {
  if(m_userProfile == nullptr || !m_security.is_initialized()) {
    return;
  }
  auto priceIncrement = m_userProfile->GetInteractionProperties().Get(
    *m_security).m_priceIncrements[static_cast<int>(modifier)];
  auto increment = static_cast<Quantity>(priceIncrement) / Quantity::MULTIPLIER;
  if(increment != m_spinBox->singleStep()) {
    m_spinBox->setSingleStep(static_cast<double>(increment));
  }
}

void MoneySpinBox::OnValueChanged(const QString& value) {
  m_valueUpdatedSignal(GetValue());
}
