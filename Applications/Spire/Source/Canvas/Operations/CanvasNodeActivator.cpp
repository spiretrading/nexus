#include "Spire/Canvas/Operations/CanvasNodeActivator.hpp"
#include <QMessageBox>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterTasksModel.hpp"
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
  vector<CanvasNodeValidationError> errors = Validate(node);
  if(!errors.empty()) {
    QMessageBox::warning(parent, QObject::tr("Error"),
      QString::fromStdString("Node contains errors."));
    return;
  }
  if(IsCompatible(OrderReferenceType::GetInstance(), node.GetType())) {
    userProfile.GetBlotterSettings().GetActiveBlotter().GetTasksModel().Add(
      node);
  }
}
