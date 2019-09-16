#ifndef SPIRE_TASKSTATENODE_HPP
#define SPIRE_TASKSTATENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/TaskNodes/Task.hpp"
#include "Spire/Canvas/Types/TaskStateType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class TaskStateNode
      \brief Implements the CanvasNode for an TaskState value.
   */
  class TaskStateNode : public ValueNode<TaskStateType> {
    public:

      //! Constructs a TaskStateNode.
      TaskStateNode();

      //! Constructs a TaskStateNode.
      /*!
        \param value The initial value.
      */
      TaskStateNode(Task::State value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<TaskStateNode> SetValue(Task::State value) const;

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
  void TaskStateNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<TaskStateType>::Shuttle(shuttle, version);
  }
}

#endif
