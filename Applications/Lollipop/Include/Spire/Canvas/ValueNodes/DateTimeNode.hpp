#ifndef SPIRE_DATETIMENODE_HPP
#define SPIRE_DATETIMENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class DateTimeNode
      \brief Implements the CanvasNode for a date/time.
   */
  class DateTimeNode : public ValueNode<DateTimeType> {
    public:

      //! Constructs a DateTimeNode.
      DateTimeNode();

      //! Constructs a DateTimeNode.
      /*!
        \param value The initial value.
      */
      DateTimeNode(const boost::posix_time::ptime& value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<DateTimeNode> SetValue(
        const boost::posix_time::ptime& value) const;

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
  void DateTimeNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<DateTimeType>::shuttle(shuttle, version);
  }
}

#endif
