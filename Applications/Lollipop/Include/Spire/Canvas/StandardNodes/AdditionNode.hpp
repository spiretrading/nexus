#ifndef SPIRE_ADDITION_NODE_HPP
#define SPIRE_ADDITION_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /** Specifies an AdditionNode's signatures. */
  struct AdditionNodeSignatures {
    using type = boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mpl::vector<double, double, double>,
      boost::mpl::vector<boost::posix_time::time_duration,
        boost::posix_time::time_duration, boost::posix_time::time_duration>,
      boost::mpl::vector<boost::posix_time::ptime,
        boost::posix_time::time_duration, boost::posix_time::ptime>,
      boost::mpl::vector<Nexus::Money, Nexus::Money, Nexus::Money>,
      boost::mpl::vector<Nexus::Quantity, double, double>,
      boost::mpl::vector<double, Nexus::Quantity, double>>;
  };

  /** Adds two CanvasNodes together. */
  class AdditionNode : public FunctionNode {
    public:

      /** Constructs an AdditionNode. */
      AdditionNode();

      void Apply(CanvasNodeVisitor& visitor) const override;

      using FunctionNode::Replace;
    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

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

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::AdditionNode> : std::false_type {};
}

#endif
