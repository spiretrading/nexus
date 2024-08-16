#ifndef SPIRE_DURATIONNODE_HPP
#define SPIRE_DURATIONNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class DurationNode
      \brief Implements the CanvasNode for a time duration.
   */
  class DurationNode : public ValueNode<DurationType> {
    public:

      //! Constructs a DurationNode.
      DurationNode();

      //! Constructs a DurationNode.
      /*!
        \param value The initial value.
      */
      DurationNode(boost::posix_time::time_duration value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<DurationNode> SetValue(
        boost::posix_time::time_duration value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void DurationNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<DurationType>::Shuttle(shuttle, version);
  }
}

#endif
