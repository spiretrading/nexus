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
      using Type = std::shared_ptr<Nexus::Order>;

      //! Returns an instance of this type.
      static const OrderReferenceType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      OrderReferenceType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OrderReferenceType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
