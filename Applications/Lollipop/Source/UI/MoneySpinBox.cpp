#include "Spire/UI/MoneySpinBox.hpp"
#include <QKeyEvent>
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

MoneySpinBox::MoneySpinBox(QWidget* parent)
    : QDoubleSpinBox(parent),
      m_userProfile(nullptr) {
  setMaximum(numeric_limits<double>::max());
  setMinimum(numeric_limits<double>::lowest());
  setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
  setKeyboardTracking(true);
  setAccelerated(false);
  setSingleStep(0.01);
  setDecimals(6);
}

MoneySpinBox::MoneySpinBox(Ref<UserProfile> userProfile,
    const MoneyNode& node, QWidget* parent)
    : QDoubleSpinBox(parent),
      m_userProfile(userProfile.get()) {
  setMaximum(numeric_limits<double>::max());
  setMinimum(numeric_limits<double>::lowest());
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

MoneySpinBox::~MoneySpinBox() {}

Money MoneySpinBox::GetValue() const {
  return parse_money(cleanText().toStdString());
}

void MoneySpinBox::SetValue(Money value) {
  setValue(static_cast<double>(static_cast<Quantity>(value)));
}

void MoneySpinBox::keyPressEvent(QKeyEvent* event) {
  AdjustIncrement(KeyModifiersFromEvent(*event));
  QDoubleSpinBox::keyPressEvent(event);
}

void MoneySpinBox::keyReleaseEvent(QKeyEvent* event) {
  if(event->modifiers() == Qt::SHIFT || event->modifiers() == Qt::ALT ||
      event->modifiers() == Qt::CTRL) {
    AdjustIncrement(KeyModifiers::PLAIN);
  }
  QDoubleSpinBox::keyReleaseEvent(event);
}

void MoneySpinBox::AdjustIncrement(KeyModifiers modifier) {
  if(m_userProfile == nullptr || !m_security.is_initialized()) {
    return;
  }
  auto priceIncrement = m_userProfile->GetInteractionProperties().get(
    *m_security).m_priceIncrements[static_cast<int>(modifier)];
  auto increment = static_cast<double>(static_cast<Quantity>(priceIncrement));
  if(increment != singleStep()) {
    setSingleStep(increment);
  }
}
