#ifndef NEXUS_MOCKORDERTASK_HPP
#define NEXUS_MOCKORDERTASK_HPP
#include <Beam/Tasks/BasicTask.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Threading/TaskQueue.hpp>
#include "Nexus/OrderTasksTests/OrderTasksTests.hpp"

namespace Nexus {
namespace OrderTasks {
namespace Tests {

  /*! \class MockOrderTask
      \brief Implements an Order Task for testing purposes.
   */
  class MockOrderTask : public Beam::Tasks::BasicTask {
    public:

      //! Constructs a MockOrderTask.
      /*!
        \param factory The MockOrderTaskFactory that was used to build this
               instance.
      */
      MockOrderTask(const MockOrderTaskFactory& factory);

      //! Returns the MockOrderTaskFactory used to build this instance.
      const MockOrderTaskFactory& GetFactory() const;

      using Beam::Tasks::BasicTask::SetActive;
      using Beam::Tasks::BasicTask::SetTerminal;

    protected:
      virtual void OnExecute();

      virtual void OnCancel();

    private:
      std::unique_ptr<MockOrderTaskFactory> m_factory;
  };

  /*! \class MockOrderTaskFactory
      \brief Implements the TaskFactory for the MockOrderTask.
   */
  class MockOrderTaskFactory : public Beam::Tasks::BasicTaskFactory<
      MockOrderTaskFactory> {
    public:

      //! The list of MockOrderTasks executed.
      typedef Beam::Threading::Sync<std::vector<MockOrderTask*>>
        MockOrderList;

      //! Constructs a MockOrderTaskFactory.
      MockOrderTaskFactory();

      //! Copies a MockOrderTaskFactory.
      /*!
        \param factory The MockOrderTaskFactory to copy.
      */
      MockOrderTaskFactory(const MockOrderTaskFactory& factory);

      //! Returns the list of all MockOrderTasks executed.
      MockOrderList& GetMockOrderList();

      virtual std::unique_ptr<Beam::Tasks::Task> Create();

      virtual void PrepareContinuation(const Beam::Tasks::Task& task);

      using Beam::Tasks::BasicTaskFactory<MockOrderTaskFactory>::
        DefineProperty;

    private:
      friend class MockOrderTask;
      std::shared_ptr<MockOrderList> m_mockOrderTasks;
  };

  inline MockOrderTask::MockOrderTask(const MockOrderTaskFactory& factory)
      : m_factory(static_cast<MockOrderTaskFactory*>(
          factory.Clone().release())) {
    m_factory->m_mockOrderTasks.reset();
  }

  inline const MockOrderTaskFactory& MockOrderTask::GetFactory() const {
    return *m_factory;
  }

  inline void MockOrderTask::OnExecute() {}

  inline void MockOrderTask::OnCancel() {}

  inline MockOrderTaskFactory::MockOrderTaskFactory()
      : m_mockOrderTasks(std::make_shared<MockOrderList>()) {}

  inline MockOrderTaskFactory::MockOrderTaskFactory(
      const MockOrderTaskFactory& factory)
      : Beam::Tasks::BasicTaskFactory<MockOrderTaskFactory>(factory),
        m_mockOrderTasks(factory.m_mockOrderTasks) {}

  inline MockOrderTaskFactory::MockOrderList&
      MockOrderTaskFactory::GetMockOrderList() {
    return *m_mockOrderTasks;
  }

  inline std::unique_ptr<Beam::Tasks::Task> MockOrderTaskFactory::Create() {
    std::unique_ptr<MockOrderTask> task = Beam::MakeUniquePtr<MockOrderTask>(
      Beam::Ref(*this));
    Beam::Threading::With(*m_mockOrderTasks,
      [&] (MockOrderList::Value& value) {
        value.push_back(task.get());
      });
    return task;
  }

  inline void MockOrderTaskFactory::PrepareContinuation(
      const Beam::Tasks::Task& task) {}
}
}
}

#endif
