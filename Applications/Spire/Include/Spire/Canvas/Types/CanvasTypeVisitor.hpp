#ifndef SPIRE_CANVAS_TYPE_VISITOR_HPP
#define SPIRE_CANVAS_TYPE_VISITOR_HPP
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Implements a single dispatch visitor pattern for CanvasTypes. */
  class CanvasTypeVisitor {
    public:
      virtual ~CanvasTypeVisitor() = default;

      virtual void Visit(const BooleanType& type);

      virtual void Visit(const CanvasType& type);

      virtual void Visit(const CurrencyType& type);

      virtual void Visit(const DateTimeType& type);

      virtual void Visit(const DecimalType& type);

      virtual void Visit(const DestinationType& type);

      virtual void Visit(const DurationType& type);

      virtual void Visit(const IntegerType& type);

      virtual void Visit(const MoneyType& type);

      virtual void Visit(const NativeType& type);

      virtual void Visit(const OrderReferenceType& type);

      virtual void Visit(const OrderStatusType& type);

      virtual void Visit(const OrderTypeType& type);

      virtual void Visit(const RecordType& type);

      virtual void Visit(const SecurityType& type);

      virtual void Visit(const SequenceType& type);

      virtual void Visit(const SideType& type);

      virtual void Visit(const TextType& type);

      virtual void Visit(const TimeInForceType& type);

      virtual void Visit(const TimeRangeType& type);

      virtual void Visit(const UnionType& type);

      virtual void Visit(const VenueType& type);
  };
}

#endif
