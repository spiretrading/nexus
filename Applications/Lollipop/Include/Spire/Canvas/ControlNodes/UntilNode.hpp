#ifndef SPIRE_UNTIL_NODE_HPP
#define SPIRE_UNTIL_NODE_HPP
#include <boost/mpl/vector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /** Evaluates a CanvasNode until a condition is true. */
  class UntilNode : public FunctionNode {
    public:

      /** Specifies an UntilNode's signatures. */
      struct Signatures {
        template<typename T>
        struct MakeSignature {
          using type = typename boost::mpl::vector<bool, T, T>::type;
        };

        using type = boost::mpl::transform<NativeTypes,
          MakeSignature<boost::mpl::placeholders::_1>>::type;
      };

      /** Constructs an UntilNode. */
      UntilNode();

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void UntilNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
