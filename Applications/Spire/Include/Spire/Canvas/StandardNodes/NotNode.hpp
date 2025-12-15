#ifndef SPIRE_NOTNODE_HPP
#define SPIRE_NOTNODE_HPP
#include <boost/mp11.hpp>
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an NotNode's signatures.
  struct NotNodeSignatures {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<bool, bool>,
      boost::mp11::mp_list<Nexus::Side, Nexus::Side>>;
  };

  /*! \class NotNode
      \brief Returns the negation of a CanvasNode.
   */
  class NotNode : public FunctionNode {
    public:

      //! Constructs a NotNode.
      NotNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void NotNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
