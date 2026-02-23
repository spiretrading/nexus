#ifndef SPIRE_TICKER_TYPE_HPP
#define SPIRE_TICKER_TYPE_HPP
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class TickerType
      \brief Represents a Ticker.
   */
  class TickerType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::Ticker Type;

      //! Returns an instance of this type.
      static const TickerType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      TickerType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TickerType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
