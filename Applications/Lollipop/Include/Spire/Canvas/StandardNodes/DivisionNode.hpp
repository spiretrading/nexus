#ifndef SPIRE_DIVISIONNODE_HPP
#define SPIRE_DIVISIONNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a DivisionNode's signatures.
  struct DivisionNodeSignatures {
    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mpl::vector<double, double, double>,
      boost::mpl::vector<Nexus::Money, Nexus::Money, double>,
      boost::mpl::vector<boost::posix_time::time_duration, Nexus::Quantity,
        boost::posix_time::time_duration>,
      boost::mpl::vector<boost::posix_time::time_duration,
        boost::posix_time::time_duration, double>,
      boost::mpl::vector<Nexus::Quantity, double, Nexus::Quantity>,
      boost::mpl::vector<double, Nexus::Quantity, double>,
      boost::mpl::vector<Nexus::Money, Nexus::Quantity, Nexus::Money>,
      boost::mpl::vector<Nexus::Money, double, Nexus::Money>> type;
  };

  /*! \class DivisionNode
      \brief Divides two CanvasNodes.
   */
  class DivisionNode : public FunctionNode {
    public:

      //! Constructs a DivisionNode.
      DivisionNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      DivisionNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void DivisionNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::DivisionNode> : std::false_type {};
}
}

#endif
