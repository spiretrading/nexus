#include "Spire/Canvas/Operations/CanvasNodeActivator.hpp"
#include <QMessageBox>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

void Spire::Activate(const CanvasNode& node, UserProfile& userProfile,
    QWidget* parent) {
  auto errors = Validate(node);
  if(!errors.empty()) {
    QMessageBox::warning(parent, QObject::tr("Unable to Add Task"),
      QString::fromStdString(errors.front().GetErrorMessage()));
    return;
  }
  if(!IsCompatible(OrderReferenceType::GetInstance(), node.GetType())) {
    return;
  }
  try {
    auto& registry = userProfile.GetBlotterRegistry();
    registry.get(*registry.get_active()->get()).add(node);
  } catch(const std::exception& e) {
    QMessageBox::warning(parent, QObject::tr("Unable to Add Task"),
      QString::fromStdString(e.what()));
  }
}
