#ifndef SPIRE_MULTIPLICATIONNODE_HPP
#define SPIRE_MULTIPLICATIONNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a MultiplicationNode's signatures.
  struct MultiplicationNodeSignatures {
    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, Nexus::Money, Nexus::Money>,
      boost::mpl::vector<Nexus::Money, Nexus::Quantity, Nexus::Money>,
      boost::mpl::vector<double, Nexus::Money, Nexus::Money>,
      boost::mpl::vector<Nexus::Money, double, Nexus::Money>,
      boost::mpl::vector<Nexus::Quantity, boost::posix_time::time_duration,
        boost::posix_time::time_duration>,
      boost::mpl::vector<boost::posix_time::time_duration, Nexus::Quantity,
        boost::posix_time::time_duration>,
      boost::mpl::vector<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mpl::vector<double, double, double>,
      boost::mpl::vector<Nexus::Quantity, double, double>,
      boost::mpl::vector<double, Nexus::Quantity, double>> type;
  };

  /*! \class MultiplicationNode
      \brief Multiplies two CanvasNodes together.
   */
  class MultiplicationNode : public FunctionNode {
    public:

      //! Constructs a MultiplicationNode.
      MultiplicationNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      MultiplicationNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void MultiplicationNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::MultiplicationNode> : std::false_type {};
}
}

#endif
