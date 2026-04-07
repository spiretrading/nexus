#ifndef SPIRE_IFNODE_HPP
#define SPIRE_IFNODE_HPP
#include <boost/mp11.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an IfNode's signatures.
  struct IfNodeSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<bool, T, T, T>;

    using type = boost::mp11::mp_transform<MakeSignature, NativeTypes>;
  };

  /*! \class IfNode
      \brief Evaluates to one of two CanvasNodes depending on a condition.
   */
  class IfNode : public FunctionNode {
    public:

      //! Constructs an IfNode.
      IfNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void IfNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
