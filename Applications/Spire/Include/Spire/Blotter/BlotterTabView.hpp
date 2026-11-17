#ifndef SPIRE_BLOTTER_TAB_VIEW_HPP
#define SPIRE_BLOTTER_TAB_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/BlotterTaskEntry.hpp"

namespace Spire {

  /** Displays a blotter's secondary views. */
  class BlotterTabView : public QWidget {
    public:

      /**
       * Constructs a BlotterTabView.
       * @param tasks The blotter's tasks.
       * @param parent The parent widget.
       */
      explicit BlotterTabView(
        std::shared_ptr<TasksModel> tasks, QWidget* parent = nullptr);
  };
}

#endif
