#ifndef SPIRE_TEXTTYPE_HPP
#define SPIRE_TEXTTYPE_HPP
#include <string>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class TextType
      \brief Represents a text type.
   */
  class TextType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef std::string Type;

      //! Returns an instance of this type.
      static const TextType& GetInstance();

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      TextType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TextType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
