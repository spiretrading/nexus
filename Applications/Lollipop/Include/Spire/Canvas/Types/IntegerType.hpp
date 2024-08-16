#ifndef SPIRE_INTEGERTYPE_HPP
#define SPIRE_INTEGERTYPE_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class IntegerType
      \brief Represents a integer Quantity.
   */
  class IntegerType : public NativeType {
    public:

      //! Defines the native type being represented.
      using Type = Nexus::Quantity;

      //! Returns an instance of this type.
      static const IntegerType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      IntegerType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void IntegerType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
