#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/StandardNodes/TimeRangeParameterNode.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MarketNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  class DefaultCanvasNodeFromCanvasTypeVisitor : private CanvasTypeVisitor {
    public:
      std::unique_ptr<CanvasNode> GetNode(const CanvasType& type);

    private:
      std::unique_ptr<CanvasNode> m_node;

      virtual void Visit(const BooleanType& type);
      virtual void Visit(const CanvasType& type);
      virtual void Visit(const CurrencyType& type);
      virtual void Visit(const DateTimeType& type);
      virtual void Visit(const DecimalType& type);
      virtual void Visit(const DestinationType& type);
      virtual void Visit(const DurationType& type);
      virtual void Visit(const IntegerType& type);
      virtual void Visit(const MarketType& type);
      virtual void Visit(const MoneyType& type);
      virtual void Visit(const OrderStatusType& type);
      virtual void Visit(const OrderTypeType& type);
      virtual void Visit(const SecurityType& type);
      virtual void Visit(const SideType& type);
      virtual void Visit(const TextType& type);
      virtual void Visit(const TimeInForceType& type);
      virtual void Visit(const TimeRangeType& type);
  };
}

unique_ptr<CanvasNode> DefaultCanvasNodeFromCanvasTypeVisitor::GetNode(
    const CanvasType& type) {
  type.Apply(*this);
  return move(m_node);
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const BooleanType& type) {
  m_node = make_unique<BooleanNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const CanvasType& type) {
  m_node = make_unique<NoneNode>(type);
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const CurrencyType& type) {
  m_node = make_unique<CurrencyNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const DateTimeType& type) {
  m_node = make_unique<DateTimeNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const DecimalType& type) {
  m_node = make_unique<DecimalNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(
    const DestinationType& type) {
  m_node = make_unique<DestinationNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const DurationType& type) {
  m_node = make_unique<DurationNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const IntegerType& type) {
  m_node = make_unique<IntegerNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const MarketType& type) {
  m_node = make_unique<MarketNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const MoneyType& type) {
  m_node = make_unique<MoneyNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(
    const OrderStatusType& type) {
  m_node = make_unique<OrderStatusNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const OrderTypeType& type) {
  m_node = make_unique<OrderTypeNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const SecurityType& type) {
  m_node = make_unique<SecurityNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const SideType& type) {
  m_node = make_unique<SideNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const TextType& type) {
  m_node = make_unique<TextNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(
    const TimeInForceType& type) {
  m_node = make_unique<TimeInForceNode>();
}

void DefaultCanvasNodeFromCanvasTypeVisitor::Visit(const TimeRangeType& type) {
  m_node = make_unique<TimeRangeParameterNode>();
}

unique_ptr<CanvasNode> Spire::MakeDefaultCanvasNode(const CanvasType& type) {
  DefaultCanvasNodeFromCanvasTypeVisitor visitor;
  return visitor.GetNode(type);
}
