#ifndef SPIRE_CANVASNODETASK_HPP
#define SPIRE_CANVASNODETASK_HPP
#include <Beam/Reactors/Trigger.hpp>
#include <Beam/SignalHandling/ScopedSlotAdaptor.hpp>
#include <Beam/Tasks/BasicTask.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class CanvasNodeTask
      \brief Represents the top level Task translated from a CanvasNode.
   */
  class CanvasNodeTask : public Beam::Tasks::BasicTask {
    public:

      //! Constructs a CanvasNodeTask.
      /*!
        \param context The context to translate the <i>node</i> in.
        \param node The CanvasNode to translate into a Task.
        \param orderExecutionPublisher Reports this Task's Order executions.
      */
      CanvasNodeTask(Beam::RefType<CanvasNodeTranslationContext> context,
        Beam::RefType<const CanvasNode> node,
        const std::shared_ptr<SpireAggregateOrderExecutionPublisher>&
        orderExecutionPublisher);

    protected:
      virtual void OnExecute();

      virtual void OnCancel();

    private:
      CanvasNodeTranslationContext m_context;
      const CanvasNode* m_node;
      Beam::Tasks::TaskFactory m_taskFactory;
      std::shared_ptr<Beam::Tasks::Task> m_task;
      std::shared_ptr<Beam::Publisher<
        const Nexus::OrderExecutionService::Order*>> m_publisher;
      State m_taskState;
      Beam::Reactors::Trigger m_trigger;
      int m_state;
      Beam::SignalHandling::ScopedSlotAdaptor m_callbacks;

      void OnTaskUpdate(const StateEntry& update);
      bool C0();
      void S0();
      void S1();
      void S2(const std::string& message);
      void S3();
      void S4(StateEntry state);
      void S5();
  };

  /*! \class CanvasNodeTaskFactory
      \brief Implements the TaskFactory for the CanvasNodeTask.
   */
  class CanvasNodeTaskFactory :
      public Beam::Tasks::BasicTaskFactory<CanvasNodeTaskFactory> {
    public:

      //! Constructs a CanvasNodeTaskFactory.
      /*!
        \param context The context to translate the <i>node</i> in.
        \param node The CanvasNode to translate into an Task.
      */
      CanvasNodeTaskFactory(Beam::RefType<CanvasNodeTranslationContext> context,
        Beam::RefType<const CanvasNode> node);

      //! Returns the OrderExecutionMonitor.
      std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
        GetOrderExecutionPublisher() const;

      virtual std::shared_ptr<Beam::Tasks::Task> Create();

      virtual boost::any& FindProperty(const std::string& name);

    private:
      CanvasNodeTranslationContext* m_context;
      std::shared_ptr<SpireAggregateOrderExecutionPublisher>
        m_orderExecutionPublisher;
      const CanvasNode* m_node;
  };

  //! Builds a SpireAggregateOrderExecutionPublisher.
  std::shared_ptr<SpireAggregateOrderExecutionPublisher>
    MakeAggregateOrderExecutionPublisher();
}

#endif
