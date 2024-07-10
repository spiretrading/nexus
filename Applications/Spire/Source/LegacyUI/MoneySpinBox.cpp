#include "Spire/LegacyUI/MoneySpinBox.hpp"
#include <QKeyEvent>
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

MoneySpinBox::MoneySpinBox(QWidget* parent)
    : QDoubleSpinBox(parent),
      m_userProfile(nullptr) {
  setMaximum(std::numeric_limits<double>::max());
  setMinimum(std::numeric_limits<double>::lowest());
  setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  setKeyboardTracking(true);
  setAccelerated(false);
  setSingleStep(0.01);
  setDecimals(6);
}

MoneySpinBox::MoneySpinBox(Ref<UserProfile> userProfile,
    const MoneyNode& node, QWidget* parent)
    : QDoubleSpinBox(parent),
      m_userProfile(userProfile.Get()) {
  setMaximum(std::numeric_limits<double>::max());
  setMinimum(std::numeric_limits<double>::lowest());
  setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  setKeyboardTracking(true);
  setAccelerated(false);
  setSingleStep(0.01);
  setDecimals(6);
  auto referent = node.FindReferent();
  if(referent) {
    if(auto anchor = FindAnchor(*referent)) {
      referent = anchor;
    }
  }
  if(referent) {
    if(auto securityValueNode = dynamic_cast<const SecurityNode*>(&*referent)) {
      m_security = securityValueNode->GetValue();
    }
  }
  AdjustIncrement(Qt::NoModifier);
}

MoneySpinBox::~MoneySpinBox() = default;

Money MoneySpinBox::GetValue() const {
  auto value = Money::FromValue(cleanText().toStdString());
  assert((value));
  return *value;
}

void MoneySpinBox::SetValue(Money value) {
  setValue(static_cast<double>(static_cast<Quantity>(value)));
}

void MoneySpinBox::keyPressEvent(QKeyEvent* event) {
  AdjustIncrement(to_modifier(event->modifiers()));
  QDoubleSpinBox::keyPressEvent(event);
}

void MoneySpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(Qt::NoModifier);
  }
  QDoubleSpinBox::keyReleaseEvent(event);
}

void MoneySpinBox::AdjustIncrement(Qt::KeyboardModifier modifier) {
  if(!m_userProfile || !m_security) {
    return;
  }
  auto priceIncrement =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      *m_security)->get_price_increment(modifier)->get();
  if(modifier == Qt::KeyboardModifier::ControlModifier) {
    priceIncrement /= 10;
  }
  auto increment = static_cast<double>(static_cast<Quantity>(priceIncrement));
  if(increment != singleStep()) {
    setSingleStep(increment);
  }
}
