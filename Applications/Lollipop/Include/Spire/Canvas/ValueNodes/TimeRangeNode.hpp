#ifndef SPIRE_TIMERANGENODE_HPP
#define SPIRE_TIMERANGENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class TimeRangeNode
      \brief Implements the CanvasNode for a time range.
   */
  class TimeRangeNode : public ValueNode<TimeRangeType> {
    public:

      //! Constructs a TimeRangeNode.
      TimeRangeNode();

      //! Constructs a TimeRangeNode.
      /*!
        \param value The initial value.
      */
      TimeRangeNode(const Beam::Queries::Range& timeRange);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<TimeRangeNode> SetValue(
        const Beam::Queries::Range& value) const;

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
  void TimeRangeNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<TimeRangeType>::Shuttle(shuttle, version);
  }
}

#endif
