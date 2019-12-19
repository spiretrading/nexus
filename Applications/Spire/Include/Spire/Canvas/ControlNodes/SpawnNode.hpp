#ifndef SPIRE_SPAWNNODE_HPP
#define SPIRE_SPAWNNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class SpawnNode
      \brief Spawns CanvasNodes upon a trigger.
   */
  class SpawnNode : public CanvasNode {
    public:

      //! Constructs a SpawnNode.
      SpawnNode();

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      SpawnNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void SpawnNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::SpawnNode> : std::false_type {};
}
}

#endif
