#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MarketType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SequenceType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Spire;

void CanvasTypeVisitor::Visit(const BooleanType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const CanvasType& type) {}

void CanvasTypeVisitor::Visit(const CurrencyType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const DateTimeType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const DecimalType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const DestinationType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const DurationType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const IntegerType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const MarketType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const MoneyType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const NativeType& type) {
  Visit(static_cast<const CanvasType&>(type));
}

void CanvasTypeVisitor::Visit(const OrderReferenceType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const OrderStatusType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const OrderTypeType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const RecordType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const SecurityType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const SequenceType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const SideType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const TextType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const TimeInForceType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const TimeRangeType& type) {
  Visit(static_cast<const NativeType&>(type));
}

void CanvasTypeVisitor::Visit(const UnionType& type) {
  Visit(static_cast<const CanvasType&>(type));
}
