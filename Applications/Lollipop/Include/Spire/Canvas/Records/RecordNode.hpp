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
      friend struct Beam::Serialization::DataShuttle;

      RecordNode() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void RecordNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

#endif
