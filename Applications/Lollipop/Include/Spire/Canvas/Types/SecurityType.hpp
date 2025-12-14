#ifndef SPIRE_SECURITYTYPE_HPP
#define SPIRE_SECURITYTYPE_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class SecurityType
      \brief Represents a Security.
   */
  class SecurityType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::Security Type;

      //! Returns an instance of this type.
      static const SecurityType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      SecurityType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SecurityType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
