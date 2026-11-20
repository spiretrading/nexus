#ifndef SPIRE_BLOTTER_TASKS_VIEW_HPP
#define SPIRE_BLOTTER_TASKS_VIEW_HPP
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Ui/TableSelectionModel.hpp"

namespace Spire {

  /** Displays a blotter's order execution tasks. */
  class BlotterTasksView : public QWidget {
    public:

      /**
       * Constructs a BlotterTasksView.
       * @param model The blotter whose tasks to display.
       * @param parent The parent widget.
       */
      explicit BlotterTasksView(
        std::shared_ptr<BlotterModel> model, QWidget* parent = nullptr);

    private:
      std::shared_ptr<TableSelectionModel> m_selection;
  };
}

#endif
