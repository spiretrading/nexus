#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/KeyBindings/InteractionsProperties.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  enum {
    TICKER = 0,
    DEFAULT_QUANTITY,
    QUANTITY_INCREMENT,
    PRICE_INCREMENT
  };
}

InteractionsNode::InteractionsNode() {
  Setup(Ticker(), DEFAULT_VENUES,
    InteractionsProperties::GetDefaultProperties());
}

InteractionsNode::InteractionsNode(Ticker ticker,
    const VenueDatabase& venueDatabase,
    const InteractionsProperties& properties) {
  Setup(std::move(ticker), venueDatabase, properties);
}

const InteractionsProperties& InteractionsNode::GetProperties() const {
  return m_properties;
}

unique_ptr<CanvasNode> InteractionsNode::Replace(const CanvasNode& child,
    unique_ptr<CanvasNode> replacement) const {
  InteractionsProperties properties = m_properties;
  if(&child == &GetChildren()[DEFAULT_QUANTITY]) {
    auto valueNode = dynamic_cast<const ValueNode<IntegerType>*>(
      replacement.get());
    if(valueNode == nullptr) {
      BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
    }
    properties.m_defaultQuantity = valueNode->GetValue();
  } else if(&child == &GetChildren()[QUANTITY_INCREMENT]) {
    auto valueNode = dynamic_cast<const ValueNode<IntegerType>*>(
      replacement.get());
    if(valueNode == nullptr) {
      BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
    }
    properties.m_quantityIncrements[0] = valueNode->GetValue();
  } else if(&child == &GetChildren()[PRICE_INCREMENT]) {
    auto valueNode = dynamic_cast<const ValueNode<MoneyType>*>(
      replacement.get());
    if(valueNode == nullptr) {
      BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
    }
    properties.m_priceIncrements[0] = valueNode->GetValue();
  } else {
    return CanvasNode::Replace(child, std::move(replacement));
  }
  auto clone = CanvasNode::Clone(*this);
  clone->SetChild(clone->GetChildren().front(), std::move(replacement));
  clone->m_properties = properties;
  return std::move(clone);
}

void InteractionsNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> InteractionsNode::Clone() const {
  return std::make_unique<InteractionsNode>(*this);
}

void InteractionsNode::Setup(Ticker ticker,
    const VenueDatabase& venueDatabase,
    const InteractionsProperties& properties) {
  m_properties = properties;
  SetText("");
  SetType(IntegerType::GetInstance());
  unique_ptr<CanvasNode> tickerNode = std::make_unique<TickerNode>(
    std::move(ticker), venueDatabase);
  tickerNode = tickerNode->SetVisible(false);
  AddChild("ticker", std::move(tickerNode));
  auto defaultQuantityNode = LinkedNode::SetReferent(
    IntegerNode(m_properties.m_defaultQuantity), "ticker");
  AddChild("default_quantity", std::move(defaultQuantityNode));
  AddChild("quantity_increment",
    std::make_unique<IntegerNode>(m_properties.m_quantityIncrements[0]));
  AddChild("price_increment",
    std::make_unique<MoneyNode>(m_properties.m_priceIncrements[0]));
}
