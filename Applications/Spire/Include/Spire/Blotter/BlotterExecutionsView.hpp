#ifndef SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#define SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/ExecutionReportsModel.hpp"

namespace Spire {

  /** Displays a blotter's executions. */
  class BlotterExecutionsView : public QWidget {
    public:

      /**
       * Constructs a BlotterExecutionsView.
       * @param reports The execution reports to display.
       * @param parent The parent widget.
       */
      explicit BlotterExecutionsView(
        std::shared_ptr<ExecutionReportsModel> reports,
        QWidget* parent = nullptr);

      /** Returns the execution reports displayed. */
      const std::shared_ptr<ExecutionReportsModel>& get_reports() const;

    private:
      std::shared_ptr<ExecutionReportsModel> m_reports;
  };
}

#endif
