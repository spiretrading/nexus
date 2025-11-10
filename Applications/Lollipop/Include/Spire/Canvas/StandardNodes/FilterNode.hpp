#ifndef SPIRE_FILTER_NODE_HPP
#define SPIRE_FILTER_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /** Used to filter out values produced by a CanvasNode. */
  class FilterNode : public SignatureNode {
    public:

      /** Constructs a FilterNode. */
      FilterNode();

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
  void FilterNode::shuttle(S& shuttle, unsigned int version) {
    SignatureNode::shuttle(shuttle, version);
  }
}

#endif
