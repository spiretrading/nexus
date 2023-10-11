#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TaskVolumeNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"

using namespace Beam;
using namespace Spire;

namespace {
  auto MakeVolumeNode() {
    auto volumeNode = MakeTaskVolumeNode();
    return volumeNode->Replace(
      volumeNode->GetChildren().back(), std::make_unique<ReferenceNode>(
        "<<target", OrderReferenceType::GetInstance()));
  }
}

BlotterTaskProperties BlotterTaskProperties::GetDefault() {
  auto properties = BlotterTaskProperties();
  auto securityMonitor =
    std::unique_ptr<CanvasNode>(std::make_unique<BlotterTaskMonitorNode>());
  securityMonitor =
    securityMonitor->Replace(securityMonitor->GetChildren().front(),
      std::make_unique<ReferenceNode>("<security"));
  properties.Add(BlotterTaskMonitor("Security", *securityMonitor));
  auto sideMonitor =
    std::unique_ptr<CanvasNode>(std::make_unique<BlotterTaskMonitorNode>());
  sideMonitor = sideMonitor->Replace(sideMonitor->GetChildren().front(),
    std::make_unique<ReferenceNode>("<side"));
  properties.Add(BlotterTaskMonitor("Side", *sideMonitor));
  auto priceMonitor =
    std::unique_ptr<CanvasNode>(std::make_unique<BlotterTaskMonitorNode>());
  priceMonitor = priceMonitor->Replace(priceMonitor->GetChildren().front(),
    std::make_unique<ReferenceNode>("<price"));
  properties.Add(BlotterTaskMonitor("Price", *priceMonitor));
  auto quantityMonitor =
    std::unique_ptr<CanvasNode>(std::make_unique<BlotterTaskMonitorNode>());
  quantityMonitor =
    quantityMonitor->Replace(quantityMonitor->GetChildren().front(),
      std::make_unique<ReferenceNode>("<quantity"));
  properties.Add(BlotterTaskMonitor("Quantity", *quantityMonitor));
  auto volumeMonitor =
    std::unique_ptr<CanvasNode>(std::make_unique<BlotterTaskMonitorNode>());
  volumeMonitor = volumeMonitor->Replace(
    volumeMonitor->GetChildren().front(), MakeVolumeNode());
  properties.Add(BlotterTaskMonitor("Volume", *volumeMonitor));
  return properties;
}

const std::vector<BlotterTaskMonitor>& BlotterTaskProperties::GetMonitors()
    const {
  return m_taskMonitors;
}

void BlotterTaskProperties::Add(const BlotterTaskMonitor& monitor) {
  m_taskMonitors.push_back(monitor);
}

void BlotterTaskProperties::Remove(const std::string& name) {
  std::erase_if(m_taskMonitors, [&] (const auto& monitor) {
    return monitor.GetName() == name;
  });
}

void BlotterTaskProperties::Replace(
    int index, const BlotterTaskMonitor& monitor) {
  m_taskMonitors[index] = monitor;
}
