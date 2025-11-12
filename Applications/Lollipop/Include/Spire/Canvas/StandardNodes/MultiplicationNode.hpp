#ifndef SPIRE_MULTIPLICATIONNODE_HPP
#define SPIRE_MULTIPLICATIONNODE_HPP
#include <boost/mp11.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a MultiplicationNode's signatures.
  struct MultiplicationNodeSignatures {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, Nexus::Money, Nexus::Money>,
      boost::mp11::mp_list<Nexus::Money, Nexus::Quantity, Nexus::Money>,
      boost::mp11::mp_list<double, Nexus::Money, Nexus::Money>,
      boost::mp11::mp_list<Nexus::Money, double, Nexus::Money>,
      boost::mp11::mp_list<Nexus::Quantity, boost::posix_time::time_duration,
        boost::posix_time::time_duration>,
      boost::mp11::mp_list<boost::posix_time::time_duration, Nexus::Quantity,
        boost::posix_time::time_duration>,
      boost::mp11::mp_list<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mp11::mp_list<double, double, double>,
      boost::mp11::mp_list<Nexus::Quantity, double, double>,
      boost::mp11::mp_list<double, Nexus::Quantity, double>>;
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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void MultiplicationNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
