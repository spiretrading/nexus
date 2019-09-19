#include "Spire/Canvas/Operations/Translation.hpp"

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
