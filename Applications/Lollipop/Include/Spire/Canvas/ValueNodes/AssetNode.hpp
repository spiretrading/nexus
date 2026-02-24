#ifndef SPIRE_ASSET_NODE_HPP
#define SPIRE_ASSET_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/AssetType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /** Implements the CanvasNode for the AssetType. */
  class AssetNode : public ValueNode<AssetType> {
    public:

      /** Constructs an AssetNode. */
      AssetNode();

      /**
       * Constructs a AssetNode with an initial value.
       * @param value The new value.
       */
      explicit AssetNode(Nexus::Asset value);

      /**
       * Clones this CanvasNode with a new value.
       * @param value The new value.
       * @return A clone of this CanvasNode with the specified <i>value</i>.
       */
      std::unique_ptr<AssetNode> SetValue(Nexus::Asset value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;
      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void AssetNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<AssetType>::shuttle(shuttle, version);
  }
}

#endif
