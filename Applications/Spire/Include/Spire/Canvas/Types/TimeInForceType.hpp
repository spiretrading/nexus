#ifndef SPIRE_TIMEINFORCETYPE_HPP
#define SPIRE_TIMEINFORCETYPE_HPP
#include "Nexus/Definitions/Definitions.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class TimeInForceType
      \brief Represents the type used for a TimeInForce.
   */
  class TimeInForceType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::TimeInForce Type;

      //! Returns an instance of this type.
      static const TimeInForceType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      TimeInForceType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeInForceType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
