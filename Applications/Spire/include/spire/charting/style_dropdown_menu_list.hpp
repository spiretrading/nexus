#ifndef SPIRE_DROPDOWN_MENU_LIST_HPP
#define SPIRE_DROPDOWN_MENU_LIST_HPP
#include <QScrollArea>
#include <QWidget>
#include "spire/charting/charting.hpp"
#include "spire/charting/trend_line.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! The list used by the TrendLineStyleDropdownMenu.
  class StyleDropdownMenuList : public QWidget {
    public:

      //! Signals that a style was selected from the list.
      using SelectedSignal = Signal<void (TrendLineStyle style)>;

      //! Constructs a StyleDropdownMenuList.
      /*
        \param parent The parent to the list.
      */
      StyleDropdownMenuList(QWidget* parent = nullptr);

      //! Returns the style in the list that follows the style specified.
      /*
        \param style The specified style.
      */
      TrendLineStyle get_next(TrendLineStyle style);

      //! Returns the style in the list that comes before the style specified.
      /*
        \param style The specified style.
      */
      TrendLineStyle get_previous(TrendLineStyle style);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      std::unique_ptr<DropShadow> m_shadow;
      QWidget* m_list_widget;
      int m_highlight_index;

      int get_index(TrendLineStyle style);
      void on_select(TrendLineStyle style);
      void focus_next();
      void focus_previous();
      void update_highlights(int old_index, int new_index);
  };
}

#endif
