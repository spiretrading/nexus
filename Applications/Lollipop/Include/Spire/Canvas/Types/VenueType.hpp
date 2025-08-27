#ifndef SPIRE_VENUETYPE_HPP
#define SPIRE_VENUETYPE_HPP
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class VenueType
      \brief Represents the type used to identify a venue.
   */
  class VenueType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::Venue Type;

      //! Returns an instance of this type.
      static const VenueType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      VenueType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void VenueType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
