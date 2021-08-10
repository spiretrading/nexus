#ifndef SPIRE_SCROLLABLE_LIST_BOX_HPP
#define SPIRE_SCROLLABLE_LIST_BOX_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Composes a ListView and a ScrollBox together to display a scrollable list
   * of values.
   */
  class ScrollableListBox : public QWidget {
    public:

      /**
       * Constructs a ScrollableListBox.
       * @param list_view The ListView to display within the ScrollBox.
       * @param parent The parent widget.
       */
      explicit ScrollableListBox(
        ListView* list_view, QWidget* parent = nullptr);

    private:
      ListView* m_list_view;
      ScrollBox* m_scroll_box;
  };
}

#endif
