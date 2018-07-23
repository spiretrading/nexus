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
      friend struct Beam::Serialization::DataShuttle;

      SecurityType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void SecurityType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
