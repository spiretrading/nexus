#ifndef SPIRE_DATETIMETYPE_HPP
#define SPIRE_DATETIMETYPE_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class DateTimeType
      \brief Represents the type used for a date and time.
   */
  class DateTimeType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef boost::posix_time::ptime Type;

      //! Returns an instance of this type.
      static const DateTimeType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      DateTimeType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DateTimeType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
