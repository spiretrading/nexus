#include "Spire/LegacyUI/OptionalPriceSpinBox.hpp"
#include <QKeyEvent>
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

OptionalPriceSpinBox::OptionalPriceSpinBox(Ref<UserProfile> userProfile,
    const OptionalPriceNode& node, QWidget* parent)
    : QDoubleSpinBox(parent),
      m_userProfile(userProfile.get()),
      m_referencePrice(node.GetReferencePrice()) {
  setMaximum(std::numeric_limits<double>::max());
  setMinimum(0);
  setSpecialValueText(tr("N/A"));
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

OptionalPriceSpinBox::~OptionalPriceSpinBox() = default;

Money OptionalPriceSpinBox::GetValue() const {
  if(auto value = try_parse_money(cleanText().toStdString())) {
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
  AdjustIncrement(to_modifier(event->modifiers()));
  QDoubleSpinBox::keyPressEvent(event);
}

void OptionalPriceSpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(Qt::NoModifier);
  }
  QDoubleSpinBox::keyReleaseEvent(event);
}

void OptionalPriceSpinBox::AdjustIncrement(Qt::KeyboardModifier modifier) {
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
