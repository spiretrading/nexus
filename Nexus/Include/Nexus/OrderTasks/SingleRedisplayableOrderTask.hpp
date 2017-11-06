#ifndef NEXUS_SINGLE_REDISPLAYABLE_ORDER_TASK_HPP
#define NEXUS_SINGLE_REDISPLAYABLE_ORDER_TASK_HPP
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Tasks/BasicTask.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/OrderTasks/OrderTasks.hpp"

namespace Nexus {
namespace OrderTasks {
namespace Details {
  template<typename Dummy>
  struct SingleRedisplayableOrderTaskProperties {

      //! The Order's quantity.
      static const std::string QUANTITY;

      //! The quantity to display.
      static const std::string DISPLAY;
  };

  template<typename Dummy>
  const std::string SingleRedisplayableOrderTaskProperties<Dummy>::QUANTITY =
    "quantity";

  template<typename Dummy>
  const std::string SingleRedisplayableOrderTaskProperties<Dummy>::DISPLAY =
    "display";
}

  /*! \class SingleRedisplayableOrderTask
      \brief Allows dynamic resizing of a Task.
   */
  class SingleRedisplayableOrderTask : public Beam::Tasks::BasicTask {
    public:

      //! Constructs a SingleRedisplayableOrderTask.
      /*!
        \param taskFactory Builds the Task to redisplay.
        \param display The quantity to display.
      */
      SingleRedisplayableOrderTask(
        const Beam::Tasks::TaskFactory& taskFactory, Quantity quantity);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      friend class SingleRedisplayableOrderTaskFactory;
      Beam::Tasks::TaskFactory m_taskFactory;
      Quantity m_quantity;
      Quantity m_currentDisplay;
      Quantity m_requestedDisplay;
      std::shared_ptr<Beam::Tasks::Task> m_task;
      State m_taskState;
      int m_state;
      Beam::RoutineTaskQueue m_tasks;

      void OnTaskUpdate(const StateEntry& update);
      bool C0();
      bool C1();
      bool C2();
      void S0();
      void S1();
      void S2(const std::string& message);
      void S3();
      void S4(State state, const std::string& message);
      void S5();
      void S6();
      void S7();
      void S8(const std::string& message);
  };

  /*! \class SingleRedisplayableOrderTaskFactory
      \brief Implements the TaskFactory for the SingleRedisplayableOrderTask.
   */
  class SingleRedisplayableOrderTaskFactory :
      public Beam::Tasks::BasicTaskFactory<
      SingleRedisplayableOrderTaskFactory>,
      private Details::SingleRedisplayableOrderTaskProperties<void> {
    public:
      using Details::SingleRedisplayableOrderTaskProperties<void>::QUANTITY;
      using Details::SingleRedisplayableOrderTaskProperties<void>::DISPLAY;

      //! Constructs a SingleRedisplayableOrderTaskFactory.
      /*!
        \param taskFactory Builds the Task to redisplay.
      */
      SingleRedisplayableOrderTaskFactory(
        const Beam::Tasks::TaskFactory& taskFactory);

      virtual std::shared_ptr<Beam::Tasks::Task> Create() override final;

      virtual boost::any& FindProperty(const std::string& name) override final;

      virtual void PrepareContinuation(
        const Beam::Tasks::Task& task) override final;

    private:
      Beam::Tasks::TaskFactory m_taskFactory;
  };

  inline SingleRedisplayableOrderTask::SingleRedisplayableOrderTask(
      const Beam::Tasks::TaskFactory& taskFactory, Quantity display)
      : m_taskFactory{taskFactory},
        m_currentDisplay{display} {}

  inline void SingleRedisplayableOrderTask::OnExecute() {
    m_tasks.Push(
      [=] {
        S0();
      });
  }

  inline void SingleRedisplayableOrderTask::OnCancel() {
    m_tasks.Push(
      [=] {
        if(m_state != 2 && m_state != 4 && m_state != 8) {
          return S4(Beam::Tasks::Task::State::CANCELED, "");
        }
      });
  }

  inline void SingleRedisplayableOrderTask::OnTaskUpdate(
      const StateEntry& update) {
    m_tasks.Push(
      [=] {
        m_taskState = update.m_state;
        if(m_state == 1) {
          if(update.m_state == State::ACTIVE) {
            return S3();
          } else if(update.m_state == State::FAILED) {
            return S2(update.m_message);
          }
        } else if(m_state == 3) {
          if(update.m_state == State::COMPLETE) {
            if(m_taskFactory->Get<Quantity>(
                SingleRedisplayableOrderTaskFactory::QUANTITY) == m_quantity) {
              return S4(update.m_state, update.m_message);
            }
          } else if(Beam::Tasks::IsTerminal(update.m_state)) {
            return S4(update.m_state, update.m_message);
          }
        } else if(m_state == 5) {
          if(update.m_state == State::CANCELED ||
              update.m_state == State::COMPLETE) {
            if(update.m_state == State::COMPLETE &&
                m_taskFactory->Get<Quantity>(
                SingleRedisplayableOrderTaskFactory::QUANTITY) == m_quantity) {
              return S4(State::COMPLETE, update.m_message);
            }
            return S6();
          } else if(Beam::Tasks::IsTerminal(update.m_state)) {
            return S4(update.m_state, update.m_message);
          }
        } else if(m_state == 6) {
          if(update.m_state == State::ACTIVE) {
            return S7();
          } else if(update.m_state == State::FAILED) {
            return S8(update.m_message);
          }
        }
      });
  }

  inline bool SingleRedisplayableOrderTask::C0() {
    return m_taskState == State::NONE || Beam::Tasks::IsTerminal(m_taskState);
  }

  inline bool SingleRedisplayableOrderTask::C1() {
    return m_currentDisplay >= m_quantity && m_requestedDisplay >= m_quantity;
  }

  inline bool SingleRedisplayableOrderTask::C2() {
    return m_currentDisplay != m_requestedDisplay;
  }

  inline void SingleRedisplayableOrderTask::S0() {
    m_state = 0;
    m_requestedDisplay = m_currentDisplay;
    m_quantity = m_taskFactory->Get<Quantity>(
      SingleRedisplayableOrderTaskFactory::QUANTITY);
    SetActive();
    return S1();
  }

  inline void SingleRedisplayableOrderTask::S1() {
    m_state = 1;
    m_taskFactory->Set(SingleRedisplayableOrderTaskFactory::QUANTITY,
      std::min(m_currentDisplay, m_quantity));
    m_taskState = State::INITIALIZING;
    m_task = m_taskFactory->Create();
    Manage(m_task);
    m_task->GetPublisher().Monitor(m_tasks.GetSlot<StateEntry>(
      std::bind(&SingleRedisplayableOrderTask::OnTaskUpdate, this,
      std::placeholders::_1)));
    m_task->Execute();
  }

  inline void SingleRedisplayableOrderTask::S2(const std::string& message) {
    m_state = 2;
    SetTerminal(State::FAILED, message);
  }

  inline void SingleRedisplayableOrderTask::S3() {
    m_state = 3;
    if(C2()) {
      if(!C1()) {
        return S5();
      }
    }
  }

  inline void SingleRedisplayableOrderTask::S4(State state,
      const std::string& message) {
    m_state = 4;
    SetTerminal(state, message);
  }

  inline void SingleRedisplayableOrderTask::S5() {
    m_state = 5;
    if(C1()) {
      return S3();
    }
    if(m_task != nullptr) {
      m_task->Cancel();
    }
    if(C0()) {
      return S6();
    }
  }

  inline void SingleRedisplayableOrderTask::S6() {
    m_state = 6;
    if(m_task != nullptr) {
      m_taskFactory->PrepareContinuation(*m_task);
    }
    m_taskFactory->Set(SingleRedisplayableOrderTaskFactory::QUANTITY,
      std::min(m_quantity, m_requestedDisplay));
    m_taskState = State::INITIALIZING;
    m_task = m_taskFactory->Create();
    Manage(m_task);
    m_task->GetPublisher().Monitor(m_tasks.GetSlot<StateEntry>(
      std::bind(&SingleRedisplayableOrderTask::OnTaskUpdate, this,
      std::placeholders::_1)));
    m_task->Execute();
  }

  inline void SingleRedisplayableOrderTask::S7() {
    m_state = 7;
    m_currentDisplay = m_requestedDisplay;
    return S3();
  }

  inline void SingleRedisplayableOrderTask::S8(const std::string& message) {
    m_state = 8;
    SetTerminal(State::FAILED, message);
  }

  inline SingleRedisplayableOrderTaskFactory::
      SingleRedisplayableOrderTaskFactory(
      const Beam::Tasks::TaskFactory& taskFactory)
      : m_taskFactory{taskFactory} {
    DefineProperty<Quantity>(DISPLAY, 0);
  }

  inline std::shared_ptr<Beam::Tasks::Task>
      SingleRedisplayableOrderTaskFactory::Create() {
    return std::make_shared<SingleRedisplayableOrderTask>(*m_taskFactory,
      Get<Quantity>(DISPLAY));
  }

  inline boost::any& SingleRedisplayableOrderTaskFactory::FindProperty(
      const std::string& name) {
    if(name == DISPLAY) {
      return BasicTaskFactory<SingleRedisplayableOrderTaskFactory>::
        FindProperty(name);
    }
    return m_taskFactory->FindProperty(name);
  }

  inline void SingleRedisplayableOrderTaskFactory::PrepareContinuation(
      const Beam::Tasks::Task& task) {
    auto& singleRedisplayableOrderTask =
      static_cast<const SingleRedisplayableOrderTask&>(task);
    Set(DISPLAY, singleRedisplayableOrderTask.m_currentDisplay);
    if(singleRedisplayableOrderTask.m_task != nullptr) {
      m_taskFactory->PrepareContinuation(*singleRedisplayableOrderTask.m_task);
    }
    m_taskFactory->Set(SingleRedisplayableOrderTaskFactory::QUANTITY,
      singleRedisplayableOrderTask.m_quantity);
  }
}
}

#endif
