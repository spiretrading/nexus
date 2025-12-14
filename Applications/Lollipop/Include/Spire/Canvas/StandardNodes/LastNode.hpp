#ifndef SPIRE_LAST_NODE_HPP
#define SPIRE_LAST_NODE_HPP
#include <boost/mp11.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a LastNode's signatures.
  struct LastNodeSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T>;

    using type = boost::mp11::mp_transform<MakeSignature, NativeTypes>;
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
