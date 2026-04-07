#include "Spire/LegacyUI/MaxFloorSpinBox.hpp"
#include <QKeyEvent>
#include <QLineEdit>
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

MaxFloorSpinBox::MaxFloorSpinBox(
    Ref<UserProfile> userProfile, const MaxFloorNode& node, QWidget* parent)
    : QSpinBox(parent),
      m_userProfile(userProfile.get()) {
  setMaximum(std::numeric_limits<int>::max());
  setMinimum(-1);
  setSpecialValueText(tr("N/A"));
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
    }
  }
  AdjustIncrement(Qt::NoModifier);
}

MaxFloorSpinBox::~MaxFloorSpinBox() = default;

void MaxFloorSpinBox::stepBy(int steps) {
  if(steps <= 0) {
    return QSpinBox::stepBy(steps);
  }
  if(value() == -1) {
    setValue(0);
    return;
  }
  QSpinBox::stepBy(steps);
}

void MaxFloorSpinBox::keyPressEvent(QKeyEvent* event) {
  if((event->modifiers() == 0 || event->modifiers() == Qt::ShiftModifier) &&
      event->key() == Qt::Key_N) {
    setValue(-1);
    return;
  }
  if(event->key() == Qt::Key_Delete) {
    setValue(-1);
    selectAll();
    return;
  }
  AdjustIncrement(to_modifier(event->modifiers()));
  QSpinBox::keyPressEvent(event);
}

void MaxFloorSpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(Qt::NoModifier);
  }
  QSpinBox::keyReleaseEvent(event);
}

void MaxFloorSpinBox::AdjustIncrement(Qt::KeyboardModifier modifier) {
  if(!m_security) {
    return;
  }
  auto quantityIncrement =
    m_userProfile->GetKeyBindings()->get_interactions_key_bindings(
      *m_security)->get_quantity_increment(modifier)->get();
  if(modifier == Qt::KeyboardModifier::ControlModifier) {
    quantityIncrement /= 10;
  }
  if(quantityIncrement != singleStep()) {
    setSingleStep(static_cast<int>(quantityIncrement));
  }
}
