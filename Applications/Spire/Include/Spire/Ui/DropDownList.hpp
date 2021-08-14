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

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      ListView* m_list_view;
      ScrollableListBox* m_scrollable_list_box;
      OverlayPanel* m_panel;
      QSize m_panel_border_size;
      mutable boost::optional<QSize> m_size_hint;

      void on_panel_style();
  };
}

#endif
