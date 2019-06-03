#ifndef SPIRE_STYLE_DROPDOWN_MENU_LIST_HPP
#define SPIRE_STYLE_DROPDOWN_MENU_LIST_HPP
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! The list used by the StyleDropDownMenu.
  class StyleDropDownMenuList : public QWidget {
    public:

      //! Signals that an item was selected from the list.
      using SelectedSignal = Signal<void (TrendLineStyle style)>;

      //! Constructs a StyleDropDownMenuList.
      /*
        \param parent The parent to the list.
      */
      StyleDropDownMenuList(QWidget* parent = nullptr);

      //! Returns the item in the list that follows the item specified.
      /*
        \param style The specified item.
      */
      TrendLineStyle get_next(TrendLineStyle style);

      //! Returns the item in the list that comes before the item specified.
      /*
        \param style The specified item.
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
