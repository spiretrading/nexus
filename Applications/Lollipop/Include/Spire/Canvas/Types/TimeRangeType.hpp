#ifndef SPIRE_TIMERANGETYPE_HPP
#define SPIRE_TIMERANGETYPE_HPP
#include <Beam/Queries/Range.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class TimeRangeType
      \brief Represents the type of time range used in a Query.
   */
  class TimeRangeType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Beam::Queries::Range Type;

      //! Returns an instance of this type.
      static const TimeRangeType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      TimeRangeType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeRangeType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
