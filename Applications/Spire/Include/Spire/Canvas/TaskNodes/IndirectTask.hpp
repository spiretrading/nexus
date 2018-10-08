#ifndef SPIRE_INDIRECTTASKFACTORY_HPP
#define SPIRE_INDIRECTTASKFACTORY_HPP
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/Reactors.hpp>
#include <Beam/SignalHandling/ScopedSlotAdaptor.hpp>
#include <Beam/Tasks/BasicTask.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class IndirectTask
      \brief Serves as a placeholder for a Task used for referencing purposes.
   */
  class IndirectTask : public Beam::Tasks::BasicTask {
    public:

      //! Constructs an IndirectTask.
      /*!
        \param reactorMonitor The ReactorMonitor used to synchronize Tasks.
      */
      IndirectTask(Beam::Ref<Beam::Reactors::ReactorMonitor>
        reactorMonitor);

      //! Sets the Task to indirectly reference.
      /*!
        \param task The Task to reference.
      */
      void SetTask(std::shared_ptr<Beam::Tasks::Task> task);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      std::shared_ptr<Task> m_task;
      Beam::Reactors::ReactorMonitor* m_reactorMonitor;
      Beam::SignalHandling::ScopedSlotAdaptor m_callbacks;

      void OnTaskUpdate(const Beam::Tasks::Task::StateEntry& update);
  };

  /*! \class IndirectTaskFactory
      \brief Implements a TaskFactory used as a placeholder for referencing
             purposes.
   */
  class IndirectTaskFactory : public Beam::Tasks::BasicTaskFactory<
      IndirectTaskFactory> {
    public:

      //! Constructs an IndirectTaskFactory.
      /*!
        \param reactorMonitor The ReactorMonitor used to synchronize Tasks.
      */
      IndirectTaskFactory(Beam::Ref<Beam::Reactors::ReactorMonitor>
        reactorMonitor);

      //! Returns the IndirectTask.
      /*!
        \param task The IndirectTask held by this factory.
      */
      const std::shared_ptr<IndirectTask>& GetTask() const;

      //! Sets the TaskFactory to indirectly reference.
      /*!
        \param taskFactory The TaskFactory to indirectly reference.
      */
      void SetTaskFactory(const Beam::Tasks::TaskFactory& taskFactory);

      virtual std::shared_ptr<Beam::Tasks::Task> Create() override final;

      virtual boost::any& FindProperty(const std::string& name) override final;

    private:
      std::shared_ptr<boost::optional<Beam::Tasks::TaskFactory>> m_taskFactory;
      std::shared_ptr<IndirectTask> m_task;
  };
}

#endif
