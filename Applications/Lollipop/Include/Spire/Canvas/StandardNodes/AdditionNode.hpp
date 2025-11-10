#ifndef SPIRE_ADDITION_NODE_HPP
#define SPIRE_ADDITION_NODE_HPP
#include <boost/mpl/vector.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void AdditionNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
