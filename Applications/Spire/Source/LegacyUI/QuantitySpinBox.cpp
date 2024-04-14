#include "Spire/LegacyUI/QuantitySpinBox.hpp"
#include <QKeyEvent>
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

QuantitySpinBox::QuantitySpinBox(Ref<UserProfile> userProfile,
    const IntegerNode& node, QWidget* parent)
    : QSpinBox(parent),
      m_userProfile(userProfile.Get()) {
  setMaximum(std::numeric_limits<int>::max());
  setMinimum(std::numeric_limits<int>::min());
  setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  setKeyboardTracking(true);
  setAccelerated(false);
  setSingleStep(1);
  auto referent = node.FindReferent();
  if(referent) {
    if(auto anchor = FindAnchor(*referent)) {
      referent = anchor;
    }
  }
  if(referent) {
    if(auto securityValueNode = dynamic_cast<const SecurityNode*>(&*referent)) {
      m_security = securityValueNode->GetValue();
      setMinimum(0);
    }
  }
  AdjustIncrement(Qt::NoModifier);
}

void QuantitySpinBox::keyPressEvent(QKeyEvent* event) {
  AdjustIncrement(to_modifier(event->modifiers()));
  QSpinBox::keyPressEvent(event);
}

void QuantitySpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(Qt::NoModifier);
  }
  QSpinBox::keyReleaseEvent(event);
}

void QuantitySpinBox::AdjustIncrement(Qt::KeyboardModifier modifier) {
  if(!m_security) {
    return;
  }
  auto quantityIncrement =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      *m_security)->get_quantity_increment(modifier)->get();
  if(quantityIncrement != singleStep()) {
    setSingleStep(static_cast<int>(quantityIncrement));
  }
}
