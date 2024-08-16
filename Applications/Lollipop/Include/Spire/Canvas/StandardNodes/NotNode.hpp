#ifndef SPIRE_NOTNODE_HPP
#define SPIRE_NOTNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an NotNode's signatures.
  struct NotNodeSignatures {
    typedef boost::mpl::list<
      boost::mpl::vector<bool, bool>,
      boost::mpl::vector<Nexus::Side, Nexus::Side>> type;
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
      friend struct Beam::Serialization::DataShuttle;

      NotNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void NotNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::NotNode> : std::false_type {};
}
}

#endif
