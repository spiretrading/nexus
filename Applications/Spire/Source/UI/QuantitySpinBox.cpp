#include "Spire/UI/QuantitySpinBox.hpp"
#include <QKeyEvent>
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

QuantitySpinBox::QuantitySpinBox(Ref<UserProfile> userProfile,
    const IntegerNode& node, QWidget* parent)
    : QSpinBox(parent),
      m_userProfile(userProfile.Get()) {
  setMaximum(numeric_limits<int>::max());
  setMinimum(numeric_limits<int>::min());
  setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  setKeyboardTracking(true);
  setAccelerated(false);
  setSingleStep(1);
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
      setMinimum(0);
    }
  }
  AdjustIncrement(KeyModifiers::PLAIN);
}

void QuantitySpinBox::keyPressEvent(QKeyEvent* event) {
  AdjustIncrement(KeyModifiersFromEvent(*event));
  QSpinBox::keyPressEvent(event);
}

void QuantitySpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(KeyModifiers::PLAIN);
  }
  QSpinBox::keyReleaseEvent(event);
}

void QuantitySpinBox::AdjustIncrement(KeyModifiers modifier) {
  if(!m_security.is_initialized()) {
    return;
  }
  auto quantityIncrement = m_userProfile->GetInteractionProperties().Get(
    *m_security).m_quantityIncrements[static_cast<int>(modifier)];
  if(quantityIncrement != singleStep()) {
    setSingleStep(static_cast<int>(quantityIncrement));
  }
}
