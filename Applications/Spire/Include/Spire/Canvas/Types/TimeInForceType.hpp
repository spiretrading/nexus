#ifndef SPIRE_TIMEINFORCETYPE_HPP
#define SPIRE_TIMEINFORCETYPE_HPP
#include "Nexus/Definitions/TimeInForce.hpp"
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
      friend struct Beam::DataShuttle;

      TimeInForceType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeInForceType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
