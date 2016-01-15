#ifndef SPIRE_SPAWNNODE_HPP
#define SPIRE_SPAWNNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /*! \class SpawnNode
      \brief Spawns CanvasNodes upon a trigger.
   */
  class SpawnNode : public SignatureNode {
    public:

      //! Constructs a SpawnNode.
      SpawnNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual const std::vector<Signature>& GetSignatures() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      SpawnNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void SpawnNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    SignatureNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::SpawnNode> : std::false_type {};
}
}

#endif
