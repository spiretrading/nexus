#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"

using namespace Spire;

BlotterTaskMonitor::BlotterTaskMonitor()
  : BlotterTaskMonitor("", NoneNode(IntegerType::GetInstance())) {}

BlotterTaskMonitor::BlotterTaskMonitor(
    const std::string& name, const CanvasNode& monitor)
  : m_name(name),
    m_monitor(CanvasNode::Clone(monitor)) {}

BlotterTaskMonitor::BlotterTaskMonitor(const BlotterTaskMonitor& monitor)
  : m_name(monitor.m_name),
    m_monitor(CanvasNode::Clone(*monitor.m_monitor)) {}

BlotterTaskMonitor& BlotterTaskMonitor::operator =(
    const BlotterTaskMonitor& monitor) {
  m_name = monitor.m_name;
  m_monitor = CanvasNode::Clone(*monitor.m_monitor);
  return *this;
}

const std::string& BlotterTaskMonitor::GetName() const {
  return m_name;
}

const CanvasNode& BlotterTaskMonitor::GetMonitor() const {
  return *m_monitor;
}
