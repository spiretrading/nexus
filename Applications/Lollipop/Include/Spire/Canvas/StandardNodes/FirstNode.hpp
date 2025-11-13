#ifndef SPIRE_FIRST_NODE_HPP
#define SPIRE_FIRST_NODE_HPP
#include <boost/mp11.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a FirstNode's signatures.
  struct FirstNodeSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T>;

    using type = boost::mp11::mp_transform<MakeSignature, NativeTypes>;
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
