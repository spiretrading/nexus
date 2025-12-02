#ifndef SPIRE_SPAWN_NODE_HPP
#define SPIRE_SPAWN_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /** Spawns CanvasNodes upon a trigger. */
  class SpawnNode : public CanvasNode {
    public:

      /** The name of the property used to specify the trigger. */
      static const std::string TRIGGER_PROPERTY;

      /** The name of the property used to specify the series. */
      static const std::string SERIES_PROPERTY;

      /** Constructs a SpawnNode. */
      SpawnNode();

      std::unique_ptr<CanvasNode>
        Convert(const CanvasType& type) const override;

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      void Apply(CanvasNodeVisitor& visitor) const override;

      using CanvasNode::Replace;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SpawnNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
