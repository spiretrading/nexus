#ifndef SPIRE_ORDERTYPETYPE_HPP
#define SPIRE_ORDERTYPETYPE_HPP
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class OrderTypeType
      \brief Represents an OrderType's type.
   */
  class OrderTypeType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::OrderType Type;

      //! Returns an instance of this type.
      static const OrderTypeType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      OrderTypeType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OrderTypeType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
