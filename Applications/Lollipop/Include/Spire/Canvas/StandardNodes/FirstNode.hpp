#ifndef SPIRE_FIRST_NODE_HPP
#define SPIRE_FIRST_NODE_HPP
#include <boost/mpl/vector.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a FirstNode's signatures.
  struct FirstNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T>::type type;
    };

    typedef boost::mpl::transform<NativeTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class FirstNode
      \brief Evaluates to the first value it receives.
   */
  class FirstNode : public FunctionNode {
    public:

      //! Constructs a FirstNode.
      FirstNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void FirstNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
