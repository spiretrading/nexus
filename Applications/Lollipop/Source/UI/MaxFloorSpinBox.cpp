#include "Spire/UI/MaxFloorSpinBox.hpp"
#include <QKeyEvent>
#include <QLineEdit>
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

MaxFloorSpinBox::MaxFloorSpinBox(Ref<UserProfile> userProfile,
    const MaxFloorNode& node, QWidget* parent)
    : QSpinBox(parent),
      m_userProfile(userProfile.get()) {
  setMaximum(numeric_limits<int>::max());
  setMinimum(-1);
  setSpecialValueText(tr("N/A"));
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
    auto tickerValueNode = dynamic_cast<const TickerNode*>(&*referent);
    if(tickerValueNode != nullptr) {
      m_ticker = tickerValueNode->GetValue();
    }
  }
  AdjustIncrement(KeyModifiers::PLAIN);
}

MaxFloorSpinBox::~MaxFloorSpinBox() {}

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
  AdjustIncrement(KeyModifiersFromEvent(*event));
  QSpinBox::keyPressEvent(event);
}

void MaxFloorSpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(KeyModifiers::PLAIN);
  }
  QSpinBox::keyReleaseEvent(event);
}

void MaxFloorSpinBox::AdjustIncrement(KeyModifiers modifier) {
  if(!m_ticker.is_initialized()) {
    return;
  }
  auto quantityIncrement = m_userProfile->GetInteractionProperties().get(
    *m_ticker).m_quantityIncrements[static_cast<int>(modifier)];
  if(quantityIncrement != singleStep()) {
    setSingleStep(static_cast<int>(quantityIncrement));
  }
}
