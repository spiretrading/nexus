#ifndef SPIRE_ORDER_REFERENCE_TYPE_HPP
#define SPIRE_ORDER_REFERENCE_TYPE_HPP
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /** Represents an Order. */
  class OrderReferenceType : public NativeType {
    public:

      //! Defines the native type being represented.
      using Type = const Nexus::OrderExecutionService::Order*;

      //! Returns an instance of this type.
      static const OrderReferenceType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      OrderReferenceType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void OrderReferenceType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
