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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void PreviousNode::shuttle(S& shuttle, unsigned int version) {
    SignatureNode::shuttle(shuttle, version);
  }
}

#endif
