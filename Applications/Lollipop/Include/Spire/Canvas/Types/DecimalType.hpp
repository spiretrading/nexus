#ifndef SPIRE_DECIMALTYPE_HPP
#define SPIRE_DECIMALTYPE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class DecimalType
      \brief Represents a floating point/decimal type.
   */
  class DecimalType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef double Type;

      //! Returns an instance of this type.
      static const DecimalType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      DecimalType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DecimalType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
