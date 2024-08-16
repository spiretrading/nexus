#ifndef SPIRE_FUNCTIONALACTION_HPP
#define SPIRE_FUNCTIONALACTION_HPP
#include <functional>
#include <QAction>
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class FunctionalAction
      \brief Represents a QAction that executes an std::function.
   */
  class FunctionalAction : public QAction {
    public:

      //! Constructs a FunctionalAction.
      /*!
        \param function The function to execute.
        \param parent The parent this action belongs to.
      */
      FunctionalAction(const std::function<void ()>& function,
        QObject* parent = nullptr);

      //! Constructs a FunctionalAction.
      /*!
        \param parent The parent this action belongs to.
      */
      FunctionalAction(QObject* parent = nullptr);

      virtual ~FunctionalAction();

      //! Executes the function.
      void Execute();

      //! Sets the function to execute.
      /*!
        \param function The function to execute.
      */
      void SetFunction(const std::function<void ()>& function);

    private:
      std::function<void ()> m_function;
  };
}
}

#endif
