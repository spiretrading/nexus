#ifndef SPIRE_ORDEREXECTIONPUBLISHERTASK_HPP
#define SPIRE_ORDEREXECTIONPUBLISHERTASK_HPP
#include <Beam/Tasks/BasicTask.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class OrderExecutionPublisherTaskFactory
      \brief Maintains shared ownership of an OrderExecutionPublisher.
   */
  class OrderExecutionPublisherTaskFactory :
      public Beam::Tasks::BasicTaskFactory<OrderExecutionPublisherTaskFactory> {
    public:

      //! Constructs an OrderExecutionPublisherTaskFactory.
      /*!
        \param taskFactory The TaskFactory to wrap.
        \param publisher The Publisher to maintain ownership of.
      */
      OrderExecutionPublisherTaskFactory(
        const Beam::Tasks::TaskFactory& taskFactory,
        std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
        publisher);

      virtual std::shared_ptr<Beam::Tasks::Task> Create();

      virtual boost::any& FindProperty(const std::string& name);

      virtual void PrepareContinuation(const Beam::Tasks::Task& task);

    private:
      Beam::Tasks::TaskFactory m_taskFactory;
      std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
        m_publisher;
  };
}

#endif
