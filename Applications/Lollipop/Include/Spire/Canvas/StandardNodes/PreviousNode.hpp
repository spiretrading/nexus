#ifndef SPIRE_PREVIOUS_NODE_HPP
#define SPIRE_PREVIOUS_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /** Used to produce the previous value evaluated by its child. */
  class PreviousNode : public SignatureNode {
    public:

      /** Constructs a PreviousNode. */
      PreviousNode();

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

      const std::vector<Signature>& GetSignatures() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;

      PreviousNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void PreviousNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    SignatureNode::Shuttle(shuttle, version);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::PreviousNode> : std::false_type {};
}

#endif
