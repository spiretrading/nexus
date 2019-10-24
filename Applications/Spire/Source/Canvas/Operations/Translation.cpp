#include "Spire/Canvas/Operations/Translation.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

const std::type_info& Translation::GetTypeInfo() const {
  return typeid(int);
}

const Publisher<const Order*>* Translation::GetPublisher() const {
  return nullptr;
}

#if 0
  /*! \struct TaskTranslation
      \brief Stores the CanvasNode translation of a Task.
   */
  struct TaskTranslation {

    //! The factory that builds the Task.
    Aspen::Shared<Task> m_task;

    //! The OrderExecutionPublisher for all Orders executed by the factory.
    std::shared_ptr<Nexus::OrderExecutionService::OrderExecutionPublisher>
      m_publisher;
  };

  //! Defines the types of canvas translations.
  typedef boost::variant<TaskTranslation, Aspen::Box<void>> Translation;

#endif
