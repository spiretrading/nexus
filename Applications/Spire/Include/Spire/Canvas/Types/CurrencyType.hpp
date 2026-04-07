#ifndef SPIRE_CURRENCYTYPE_HPP
#define SPIRE_CURRENCYTYPE_HPP
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class CurrencyType
      \brief Represents a currency type.
   */
  class CurrencyType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::CurrencyId Type;

      //! Returns an instance of this type.
      static const CurrencyType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      CurrencyType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void CurrencyType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
