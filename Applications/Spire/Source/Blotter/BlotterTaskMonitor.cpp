#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TaskVolumeNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Spire;

std::shared_ptr<TaskMonitorsModel> Spire::make_default_task_monitors() {
  auto monitors = std::make_shared<ArrayListModel<BlotterTaskMonitor>>();
  auto add = [&] (
      QString name, int width, std::unique_ptr<CanvasNode> expression) {
    auto node = BlotterTaskMonitorNode();
    monitors->push(BlotterTaskMonitor(std::move(name),
      node.Replace(node.GetChildren().front(), std::move(expression)), width));
  };
  add(QObject::tr("Ticker"), 90, std::make_unique<ReferenceNode>("<ticker"));
  add(QObject::tr("Side"), 50, std::make_unique<ReferenceNode>("<side"));
  add(QObject::tr("Price"), 70, std::make_unique<ReferenceNode>("<price"));
  add(QObject::tr("Quantity"), 75,
    std::make_unique<ReferenceNode>("<quantity"));
  auto volume = MakeTaskVolumeNode();
  add(QObject::tr("Volume"), 75, volume->Replace(
    volume->GetChildren().back(), std::make_unique<ReferenceNode>(
      "<<target", OrderReferenceType::GetInstance())));
  return monitors;
}
