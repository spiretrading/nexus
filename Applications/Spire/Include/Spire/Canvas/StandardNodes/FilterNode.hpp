#ifndef SPIRE_FILTERNODE_HPP
#define SPIRE_FILTERNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"

namespace Spire {

  /*! \class FilterNode
      \brief Used to filter out values produced by a CanvasNode.
   */
  class FilterNode : public SignatureNode {
    public:

      //! Constructs a FilterNode.
      FilterNode();

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual const std::vector<Signature>& GetSignatures() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      FilterNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void FilterNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    SignatureNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::FilterNode> : std::false_type {};
}
}

#endif
