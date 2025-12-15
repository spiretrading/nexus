#ifndef SPIRE_ORDERSTATUSTYPE_HPP
#define SPIRE_ORDERSTATUSTYPE_HPP
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class OrderStatusType
      \brief Represents an OrderStatus' type.
   */
  class OrderStatusType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::OrderStatus Type;

      //! Returns an instance of this type.
      static const OrderStatusType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      OrderStatusType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OrderStatusType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
