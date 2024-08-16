#ifndef SPIRE_MARKETTYPE_HPP
#define SPIRE_MARKETTYPE_HPP
#include "Nexus/Definitions/Market.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class MarketType
      \brief Represents the type used to identify a market.
   */
  class MarketType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::MarketCode Type;

      //! Returns an instance of this type.
      static const MarketType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      MarketType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void MarketType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
