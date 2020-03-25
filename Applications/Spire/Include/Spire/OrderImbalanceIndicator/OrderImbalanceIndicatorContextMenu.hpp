#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_CONTEXT_MENU_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_CONTEXT_MENU_HPP
#include <QMenu>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a context menu for the order imbalance indicator window.
  class OrderImbalanceIndicatorContextMenu : public QMenu {
    public:

      //! Signals that a menu option was selected.
      using SelectedSignal = Signal<void ()>;

      //! Signals that a column check box was toggled.
      /*
        \param is_checked True if the check box is in the checked state.
      */
      using ToggledSignal = Signal<void (bool is_checked)>;

      //! Constructs an OrderImbalanceINdicatorContextMenu.
      /*
        \param parent The parent widget.
      */
      explicit OrderImbalanceIndicatorContextMenu(QWidget* parent = nullptr);

      //! Connects a slot to the export table signal.
      boost::signals2::connection connect_export_table_signal(
        const SelectedSignal::slot_type& slot) const;

      //! Connects a slot to the export chart signal.
      boost::signals2::connection connect_export_chart_signal(
        const SelectedSignal::slot_type& slot) const;

      //! Connects a slot to the reset filters signals.
      boost::signals2::connection connect_reset_signal(
        const SelectedSignal::slot_type& slot) const;

      //! Connects a slot to the side toggle signal.
      boost::signals2::connection connect_side_toggled_signal(
        const ToggledSignal::slot_type& slot) const;

      //! Connects a slot to the size toggle signal.
      boost::signals2::connection connect_size_toggled_signal(
        const ToggledSignal::slot_type& slot) const;

      //! Connects a slot to the side reference price toggle signal.
      boost::signals2::connection connect_reference_price_toggled_signal(
        const ToggledSignal::slot_type& slot) const;

      //! Connects a slot to the date toggle signal.
      boost::signals2::connection connect_date_toggled_signal(
        const ToggledSignal::slot_type& slot) const;

      //! Connects a slot to the time toggle signal.
      boost::signals2::connection connect_time_toggled_signal(
        const ToggledSignal::slot_type& slot) const;

      //! Connects a slot to the notional value toggle signal.
      boost::signals2::connection connect_notional_value_toggled_signal(
        const ToggledSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SelectedSignal m_export_table_signal;
      mutable SelectedSignal m_export_chart_signal;
      mutable SelectedSignal m_reset_signal;
      mutable ToggledSignal m_side_signal;
      mutable ToggledSignal m_size_signal;
      mutable ToggledSignal m_reference_price_signal;
      mutable ToggledSignal m_date_signal;
      mutable ToggledSignal m_time_signal;
      mutable ToggledSignal m_notional_value_signal;
      QMenu* m_table_columns_menu;
      DropShadow* m_drop_shadow;
      DropShadow* m_sub_menu_drop_shadow;

      void add_check_box(const QString& text, ToggledSignal& signal);
      void add_menu_item(const QString& text, SelectedSignal& signal);
  };
}

#endif
