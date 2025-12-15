#ifndef SPIRE_ABSNODE_HPP
#define SPIRE_ABSNODE_HPP
#include <boost/mpl/vector.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an AbsNode's signatures.
  struct AbsNodeSignatures {
    typedef boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, Nexus::Quantity>,
      boost::mp11::mp_list<double, double>,
      boost::mp11::mp_list<Nexus::Money, Nexus::Money>> type;
  };

  /*! \class AbsNode
      \brief Evaluates the absolute value of two CanvasNodes.
   */
  class AbsNode : public FunctionNode {
    public:

      //! Constructs an AbsNode.
      AbsNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void AbsNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
