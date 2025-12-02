#ifndef SPIRE_TIMENODE_HPP
#define SPIRE_TIMENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class TimeNode
      \brief Implements the CanvasNode for a time of day.
   */
  class TimeNode : public ValueNode<DurationType> {
    public:

      //! Constructs a TimeNode.
      TimeNode();

      //! Constructs a TimeNode.
      /*!
        \param value The initial value.
      */
      TimeNode(boost::posix_time::time_duration value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<TimeNode> SetValue(
        boost::posix_time::time_duration value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<DurationType>::shuttle(shuttle, version);
  }
}

#endif
