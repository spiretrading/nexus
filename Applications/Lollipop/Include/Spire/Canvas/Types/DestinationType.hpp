#ifndef SPIRE_DESTINATIONTYPE_HPP
#define SPIRE_DESTINATIONTYPE_HPP
#include <string>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class DestinationType
      \brief Represents the type used for a date and time.
   */
  class DestinationType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef std::string Type;

      //! Returns an instance of this type.
      static const DestinationType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      DestinationType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void DestinationType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
