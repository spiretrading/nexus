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
      friend struct Beam::Serialization::DataShuttle;

      OrderTypeType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void OrderTypeType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
