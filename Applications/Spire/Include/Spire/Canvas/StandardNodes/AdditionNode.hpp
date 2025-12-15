#ifndef SPIRE_ADDITION_NODE_HPP
#define SPIRE_ADDITION_NODE_HPP
#include <boost/mp11.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /** Specifies an AdditionNode's signatures. */
  struct AdditionNodeSignatures {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mp11::mp_list<double, double, double>,
      boost::mp11::mp_list<boost::posix_time::time_duration,
        boost::posix_time::time_duration, boost::posix_time::time_duration>,
      boost::mp11::mp_list<boost::posix_time::ptime,
        boost::posix_time::time_duration, boost::posix_time::ptime>,
      boost::mp11::mp_list<Nexus::Money, Nexus::Money, Nexus::Money>,
      boost::mp11::mp_list<Nexus::Quantity, double, double>,
      boost::mp11::mp_list<double, Nexus::Quantity, double>>;
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
