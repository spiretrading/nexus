#ifndef SPIRE_BOOLEANTYPE_HPP
#define SPIRE_BOOLEANTYPE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class BooleanType
      \brief Represents a true/false boolean type.
   */
  class BooleanType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef bool Type;

      //! Returns an instance of this type.
      static const BooleanType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      BooleanType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BooleanType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
