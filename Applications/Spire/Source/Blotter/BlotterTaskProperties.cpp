#include "Spire/Blotter/BlotterTaskProperties.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TaskVolumeNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

namespace {
  unique_ptr<CanvasNode> MakeVolumeNode() {
    auto volumeNode = MakeTaskVolumeNode();
    volumeNode = volumeNode->Replace(volumeNode->GetChildren().back(),
      make_unique<ReferenceNode>("<<target",
      OrderReferenceType::GetInstance()));
    return volumeNode;
  }
}

BlotterTaskProperties BlotterTaskProperties::GetDefault() {
  BlotterTaskProperties properties;
  unique_ptr<CanvasNode> securityMonitor =
    make_unique<BlotterTaskMonitorNode>();
  securityMonitor = securityMonitor->Replace(
    securityMonitor->GetChildren().front(),
    make_unique<ReferenceNode>("<security"));
  properties.Add(BlotterTaskMonitor("Security", *securityMonitor));
  unique_ptr<CanvasNode> sideMonitor = make_unique<BlotterTaskMonitorNode>();
  sideMonitor = sideMonitor->Replace(sideMonitor->GetChildren().front(),
    make_unique<ReferenceNode>("<side"));
  properties.Add(BlotterTaskMonitor("Side", *sideMonitor));
  unique_ptr<CanvasNode> priceMonitor = make_unique<BlotterTaskMonitorNode>();
  priceMonitor = priceMonitor->Replace(priceMonitor->GetChildren().front(),
    make_unique<ReferenceNode>("<price"));
  properties.Add(BlotterTaskMonitor("Price", *priceMonitor));
  unique_ptr<CanvasNode> quantityMonitor =
    make_unique<BlotterTaskMonitorNode>();
  quantityMonitor = quantityMonitor->Replace(
    quantityMonitor->GetChildren().front(),
    make_unique<ReferenceNode>("<quantity"));
  properties.Add(BlotterTaskMonitor("Quantity", *quantityMonitor));
  unique_ptr<CanvasNode> volumeMonitor = make_unique<BlotterTaskMonitorNode>();
  volumeMonitor = volumeMonitor->Replace(volumeMonitor->GetChildren().front(),
    MakeVolumeNode());
  properties.Add(BlotterTaskMonitor("Volume", *volumeMonitor));
  return properties;
}

BlotterTaskProperties::BlotterTaskProperties() {}

BlotterTaskProperties::~BlotterTaskProperties() {}

BlotterTaskProperties::BlotterTaskProperties(
    const BlotterTaskProperties& properties)
    : m_taskMonitors(properties.m_taskMonitors) {}

BlotterTaskProperties& BlotterTaskProperties::operator =(
    const BlotterTaskProperties& properties) {
  m_taskMonitors = properties.m_taskMonitors;
  return *this;
}

const vector<BlotterTaskMonitor>& BlotterTaskProperties::GetMonitors() const {
  return m_taskMonitors;
}

void BlotterTaskProperties::Add(const BlotterTaskMonitor& monitor) {
  m_taskMonitors.push_back(monitor);
}

void BlotterTaskProperties::Remove(const string& name) {
  for(auto i = m_taskMonitors.begin(); i != m_taskMonitors.end(); ++i) {
    if(i->GetName() == name) {
      m_taskMonitors.erase(i);
      return;
    }
  }
}

void BlotterTaskProperties::Replace(int index,
    const BlotterTaskMonitor& monitor) {
  m_taskMonitors[index] = monitor;
}
