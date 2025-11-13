#ifndef SPIRE_RANGENODE_HPP
#define SPIRE_RANGENODE_HPP
#include <boost/mp11.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a RangeNode's signatures.
  struct RangeNodeSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T, T>;

    using type = boost::mp11::mp_transform<MakeSignature,
      boost::mp11::mp_list<Nexus::Quantity>>;
  };

  /*! \class RangeNode
      \brief Produces a range of values.
   */
  class RangeNode : public FunctionNode {
    public:

      //! Constructs a RangeNode.
      RangeNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void RangeNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
