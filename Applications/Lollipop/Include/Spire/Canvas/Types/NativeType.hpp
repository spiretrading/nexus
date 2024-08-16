#ifndef SPIRE_NATIVETYPE_HPP
#define SPIRE_NATIVETYPE_HPP
#include <typeinfo>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

namespace Spire {

  /*! \class NativeType
      \brief Base class for a completely defined type.
   */
  class NativeType : public CanvasType {
    public:

      //! Returns the shared_ptr managing this NativeType.
      operator std::shared_ptr<NativeType> () const;

      //! Returns the native type represented.
      virtual const std::type_info& GetNativeType() const = 0;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void NativeType::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasType::Shuttle(shuttle, version);
  }
}

#endif
