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
      friend struct Beam::Serialization::DataShuttle;

      CountNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CountNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    SignatureNode::Shuttle(shuttle, version);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::CountNode> : std::false_type {};
}

#endif
