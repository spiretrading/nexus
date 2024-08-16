#ifndef SPIRE_DURATIONTYPE_HPP
#define SPIRE_DURATIONTYPE_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class DurationType
      \brief Represents the type used for a time duration.
   */
  class DurationType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef boost::posix_time::time_duration Type;

      //! Returns an instance of this type.
      static const DurationType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      DurationType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void DurationType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
