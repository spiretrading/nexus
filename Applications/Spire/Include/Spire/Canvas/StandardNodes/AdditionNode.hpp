#ifndef SPIRE_ADDITIONNODE_HPP
#define SPIRE_ADDITIONNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an AdditionNode's signatures.
  struct AdditionNodeSignatures {
    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mpl::vector<double, double, double>,
      boost::mpl::vector<boost::posix_time::time_duration,
        boost::posix_time::time_duration, boost::posix_time::time_duration>,
      boost::mpl::vector<boost::posix_time::ptime,
        boost::posix_time::time_duration, boost::posix_time::ptime>,
      boost::mpl::vector<Nexus::Money, Nexus::Money, Nexus::Money>,
      boost::mpl::vector<Nexus::Quantity, double, double>,
      boost::mpl::vector<double, Nexus::Quantity, double>> type;
  };

  /*! \class AdditionNode
      \brief Adds two CanvasNodes together.
   */
  class AdditionNode : public FunctionNode {
    public:

      //! Constructs an AdditionNode.
      AdditionNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      AdditionNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void AdditionNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::AdditionNode> : std::false_type {};
}
}

#endif
