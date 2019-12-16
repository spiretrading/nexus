#include "Spire/UI/OptionalPriceSpinBox.hpp"
#include <QKeyEvent>
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

OptionalPriceSpinBox::OptionalPriceSpinBox(Ref<UserProfile> userProfile,
    const OptionalPriceNode& node, QWidget* parent)
    : QDoubleSpinBox(parent),
      m_userProfile(userProfile.Get()),
      m_referencePrice(node.GetReferencePrice()) {
  setMaximum(numeric_limits<double>::max());
  setMinimum(0);
  setSpecialValueText(tr("N/A"));
  setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  setKeyboardTracking(true);
  setAccelerated(false);
  setSingleStep(0.01);
  setDecimals(6);
  auto referent = node.FindReferent();
  if(referent.is_initialized()) {
    auto anchor = FindAnchor(*referent);
    if(anchor.is_initialized()) {
      referent = anchor;
    }
  }
  if(referent.is_initialized()) {
    auto securityValueNode = dynamic_cast<const SecurityNode*>(&*referent);
    if(securityValueNode != nullptr) {
      m_security = securityValueNode->GetValue();
    }
  }
  AdjustIncrement(KeyModifiers::PLAIN);
}

OptionalPriceSpinBox::~OptionalPriceSpinBox() {}

Money OptionalPriceSpinBox::GetValue() const {
  auto value = Money::FromValue(cleanText().toStdString());
  if(value.is_initialized()) {
    return *value;
  }
  return Money::ZERO;
}

void OptionalPriceSpinBox::SetValue(Money value) {
  setValue(static_cast<double>(static_cast<Quantity>(value)));
}

void OptionalPriceSpinBox::keyPressEvent(QKeyEvent* event) {
  if((event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier) &&
      event->key() == Qt::Key_N) {
    SetValue(Money::ZERO);
    return;
  }
  if(event->key() == Qt::Key_Delete) {
    SetValue(Money::ZERO);
    selectAll();
    return;
  }
  if(GetValue() == Money::ZERO) {
    if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
      SetValue(m_referencePrice);
      return;
    }
  }
  AdjustIncrement(KeyModifiersFromEvent(*event));
  QDoubleSpinBox::keyPressEvent(event);
}

void OptionalPriceSpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(KeyModifiers::PLAIN);
  }
  QDoubleSpinBox::keyReleaseEvent(event);
}

void OptionalPriceSpinBox::AdjustIncrement(KeyModifiers modifier) {
  if(m_userProfile == nullptr || !m_security.is_initialized()) {
    return;
  }
  auto priceIncrement = m_userProfile->GetInteractionProperties().Get(
    *m_security).m_priceIncrements[static_cast<int>(modifier)];
  auto increment = static_cast<double>(static_cast<Quantity>(priceIncrement));
  if(increment != singleStep()) {
    setSingleStep(increment);
  }
}
