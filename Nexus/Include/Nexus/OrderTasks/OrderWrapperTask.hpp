#ifndef NEXUS_ORDER_WRAPPER_TASK_HPP
#define NEXUS_ORDER_WRAPPER_TASK_HPP
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Tasks/BasicTask.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderTasks/OrderTasks.hpp"

namespace Nexus {
namespace OrderTasks {

  /*! \class OrderWrapperTask
      \brief Wraps an existing and already executed Order.
      \tparam OrderExecutionClientType The OrderExecutionClient to use to cancel
              the Order.
   */
  template<typename OrderExecutionClientType>
  class OrderWrapperTask : public Beam::Tasks::BasicTask {
    public:

      //! The OrderExecutionClient used to execute and cancel the Order.
      using OrderExecutionClient = OrderExecutionClientType;

      //! Constructs a OrderWrapperTask.
      /*!
        \param orderExecutionClient The OrderExecutionClient to use.
        \param order The Order to wrap.
      */
      OrderWrapperTask(Beam::RefType<OrderExecutionClient> orderExecutionClient,
        const OrderExecutionService::Order& order);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      OrderExecutionClient* m_orderExecutionClient;
      const OrderExecutionService::Order* m_order;
      bool m_cancellable;
      bool m_pendingCancel;
      int m_state;
      Beam::RoutineTaskQueue m_tasks;

      void OnExecutionReport(
        const OrderExecutionService::ExecutionReport& report);
      void S0();
      void S1(const std::string& message);
      void S2();
      void S3(OrderStatus state, const std::string& message);
      void S4();
  };

  /*! \class OrderWrapperTaskFactory
      \brief Implements the OrderTaskFactory for an OrderWrapperTask.
      \tparam OrderExecutionClientType The OrderExecutionClient used to cancel
              the Order.
   */
  template<typename OrderExecutionClientType>
  class OrderWrapperTaskFactory : public Beam::Tasks::BasicTaskFactory<
      OrderWrapperTaskFactory<OrderExecutionClientType>> {
    public:

      //! The OrderExecutionClient used to execute and cancel the Order.
      using OrderExecutionClient = OrderExecutionClientType;

      //! Constructs an OrderWrapperTaskFactory.
      /*!
        \param orderExecutionClient The OrderExecutionClient to use.
        \param order The Order to wrap.
      */
      OrderWrapperTaskFactory(
        Beam::RefType<OrderExecutionClient> orderExecutionClient,
        const OrderExecutionService::Order& order);

      virtual std::shared_ptr<Beam::Tasks::Task> Create() override final;

    private:
      OrderExecutionClient* m_orderExecutionClient;
      const OrderExecutionService::Order* m_order;
  };

  template<typename OrderExecutionClientType>
  OrderWrapperTask<OrderExecutionClientType>::OrderWrapperTask(
      Beam::RefType<OrderExecutionClient> orderExecutionClient,
      const OrderExecutionService::Order& order)
      : m_orderExecutionClient{orderExecutionClient.Get()},
        m_order{&order} {}

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::OnExecute() {
    m_tasks.Push(
      [=] {
        S0();
      });
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::OnCancel() {
    m_tasks.Push(
      [=] {
        m_pendingCancel = true;
        S4();
      });
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::OnExecutionReport(
      const OrderExecutionService::ExecutionReport& report) {
    if(report.m_status == OrderStatus::NEW) {
      m_cancellable = true;
    }
    if(m_state == 0) {
      if(report.m_status == OrderStatus::REJECTED) {
        return S1(report.m_text);
      } else if(report.m_status == OrderStatus::NEW) {
        return S2();
      }
    } else if(m_state == 2) {
      if(Nexus::IsTerminal(report.m_status)) {
        return S3(report.m_status, report.m_text);
      }
    } else if(m_state == 4) {
      if(report.m_status == OrderStatus::NEW) {
        return S4();
      } else if(Nexus::IsTerminal(report.m_status)) {
        return S3(report.m_status, report.m_text);
      }
    }
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::S0() {
    m_state = 0;
    m_cancellable = false;
    m_pendingCancel = false;
    SetActive();
    m_order->GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(
      std::bind(&OrderWrapperTask::OnExecutionReport, this,
      std::placeholders::_1)));
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::S1(
      const std::string& message) {
    m_state = 1;
    SetTerminal(State::FAILED, message);
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::S2() {
    m_state = 2;
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::S3(OrderStatus state,
      const std::string& message) {
    m_state = 3;
    if(state == OrderStatus::FILLED) {
      SetTerminal(State::COMPLETE, message);
    } else if(m_pendingCancel && state == OrderStatus::CANCELED) {
      SetTerminal(State::CANCELED, message);
    } else {
      SetTerminal(State::EXPIRED, message);
    }
  }

  template<typename OrderExecutionClientType>
  void OrderWrapperTask<OrderExecutionClientType>::S4() {
    m_state = 4;

    // TODO: Proper cancel to avoid Terminate precondition.
    if(m_cancellable) {
      m_orderExecutionClient->Cancel(*m_order);
    }
  }

  template<typename OrderExecutionClientType>
  OrderWrapperTaskFactory<OrderExecutionClientType>::OrderWrapperTaskFactory(
      Beam::RefType<OrderExecutionClient> orderExecutionClient,
      const OrderExecutionService::Order& order)
      : m_orderExecutionClient{orderExecutionClient.Get()},
        m_order{&order} {}

  template<typename OrderExecutionClientType>
  std::shared_ptr<Beam::Tasks::Task> OrderWrapperTaskFactory<
      OrderExecutionClientType>::Create() {
    return std::make_shared<OrderWrapperTask<OrderExecutionClient>>(
      Beam::Ref(*m_orderExecutionClient), *m_order);
  }
}
}

#endif
