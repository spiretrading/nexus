#ifndef SPIRE_DROP_DOWN_LIST_HPP
#define SPIRE_DROP_DOWN_LIST_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a pop-up drop down list. */
  class DropDownList : public QWidget {
    public:

      /**
       * Constructs a DropDownList.
       * @param list_view The list view which contains a list of items.
       * @param parent The parent widget. Used to determine the position of the
               DropDownList.
       */
      explicit DropDownList(ListView& list_view, QWidget* parent = nullptr);

    protected:
      bool event(QEvent* event) override;

    private:
      ListView* m_list_view;
      OverlayPanel* m_panel;
      QSize m_panel_border_size;

      void get_panel_border_size();
  };
}

#endif
