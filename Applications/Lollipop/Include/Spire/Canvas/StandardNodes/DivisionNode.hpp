#ifndef SPIRE_DIVISIONNODE_HPP
#define SPIRE_DIVISIONNODE_HPP
#include <boost/mp11.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a DivisionNode's signatures.
  struct DivisionNodeSignatures {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mp11::mp_list<double, double, double>,
      boost::mp11::mp_list<Nexus::Money, Nexus::Money, double>,
      boost::mp11::mp_list<boost::posix_time::time_duration, Nexus::Quantity,
        boost::posix_time::time_duration>,
      boost::mp11::mp_list<boost::posix_time::time_duration,
        boost::posix_time::time_duration, double>,
      boost::mp11::mp_list<Nexus::Quantity, double, Nexus::Quantity>,
      boost::mp11::mp_list<double, Nexus::Quantity, double>,
      boost::mp11::mp_list<Nexus::Money, Nexus::Quantity, Nexus::Money>,
      boost::mp11::mp_list<Nexus::Money, double, Nexus::Money>>;
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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DivisionNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
