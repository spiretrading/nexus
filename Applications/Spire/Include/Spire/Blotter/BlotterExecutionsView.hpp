#ifndef SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#define SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/ExecutionReportsModel.hpp"

namespace Spire {
  class TableView;

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
      friend class BlotterWindowSettings;
      std::shared_ptr<ExecutionReportsModel> m_reports;
      std::shared_ptr<ListModel<Nexus::OrderStatus>> m_statuses;
      TableView* m_table;
  };
}

#endif
