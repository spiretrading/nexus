#ifndef SPIRE_WHEN_NODE_HPP
#define SPIRE_WHEN_NODE_HPP
#include <boost/mp11.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /** Evaluates a CanvasNode when a condition is true. */
  class WhenNode : public FunctionNode {
    public:

      /** Specifies a WhenNode's signatures. */
      struct Signatures {
        template<typename T>
        using MakeSignature = boost::mp11::mp_list<bool, T, T>;

        using type = boost::mp11::mp_transform<MakeSignature, NativeTypes>;
      };

      /** Constructs a WhenNode. */
      WhenNode();

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void WhenNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
