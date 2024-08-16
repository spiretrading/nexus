#ifndef SPIRE_MONEYTYPE_HPP
#define SPIRE_MONEYTYPE_HPP
#include "Nexus/Definitions/Definitions.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class MoneyType
      \brief Represents a monetary value type.
   */
  class MoneyType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Nexus::Money Type;

      //! Returns an instance of this type.
      static const MoneyType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      MoneyType() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void MoneyType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
  }
}

#endif
