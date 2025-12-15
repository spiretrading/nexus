#ifndef SPIRE_SEQUENCE_TYPE_HPP
#define SPIRE_SEQUENCE_TYPE_HPP
#include <Beam/Queries/Sequence.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /** Represents a query sequence number. */
  class SequenceType : public NativeType {
    public:

      //! Defines the native type being represented.
      using Type = Beam::Sequence;

      //! Returns an instance of this type.
      static const SequenceType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      SequenceType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SequenceType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
