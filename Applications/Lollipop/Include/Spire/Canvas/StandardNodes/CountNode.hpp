#ifndef SPIRE_COUNT_NODE_HPP
#define SPIRE_COUNT_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /** Counts the number of values produced by its child. */
  class CountNode : public SignatureNode {
    public:

      /** Constructs a CountNode. */
      CountNode();

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
  void CountNode::shuttle(S& shuttle, unsigned int version) {
    SignatureNode::shuttle(shuttle, version);
  }
}

#endif
