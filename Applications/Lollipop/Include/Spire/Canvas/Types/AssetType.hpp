#ifndef SPIRE_ASSET_TYPE_HPP
#define SPIRE_ASSET_TYPE_HPP
#include "Nexus/Definitions/Asset.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /** Represents an asset type. */
  class AssetType : public NativeType {
    public:

      /** Defines the native type being represented. */
      using Type = Nexus::Asset;

      //! Returns an instance of this type.
      static const AssetType& GetInstance();

      virtual std::string GetName() const;
      virtual const std::type_info& GetNativeType() const;
      virtual Compatibility GetCompatibility(const CanvasType& type) const;
      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend struct Beam::DataShuttle;

      AssetType() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void AssetType::shuttle(S& shuttle, unsigned int version) {
    NativeType::shuttle(shuttle, version);
  }
}

#endif
