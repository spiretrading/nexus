#ifndef SPIRE_SIDETYPE_HPP
#define SPIRE_SIDETYPE_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class SideType
      \brief Represents a position's Side.
   */
  class SideType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::Side Type;

      //! Returns an instance of this type.
      static const SideType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      SideType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SideType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
