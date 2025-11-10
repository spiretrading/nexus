#ifndef SPIRE_LAST_NODE_HPP
#define SPIRE_LAST_NODE_HPP
#include <boost/mpl/vector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a LastNode's signatures.
  struct LastNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T>::type type;
    };

    typedef boost::mpl::transform<NativeTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class LastNode
      \brief Evaluates to the last value it receives.
   */
  class LastNode : public FunctionNode {
    public:

      //! Constructs a LastNode.
      LastNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LastNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
