#ifndef SPIRE_RECORDNODE_HPP
#define SPIRE_RECORDNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class RecordNode
      \brief Represents a RecordType as a CanvasNode.
   */
  class RecordNode : public CanvasNode {
    public:

      //! Constructs a RecordNode.
      /*!
        \param type The RecordType to represent.
      */
      RecordNode(const RecordType& type);

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;

      RecordNode() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void RecordNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
