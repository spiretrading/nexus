#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_CONTEXT_MENU_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_CONTEXT_MENU_HPP
#include <QMenu>
#include <QWidgetAction>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CheckBox.hpp"

namespace Spire {

  class OrderImbalanceIndicatorContextMenu : public QMenu {
    public:

      using SelectedSignal = Signal<void ()>;

      using ToggledSignal = Signal<void (bool)>;

      OrderImbalanceIndicatorContextMenu(QWidget* parent = nullptr);

    private:
      mutable SelectedSignal m_export_table_signal;
      mutable SelectedSignal m_export_chart_signal;
      mutable SelectedSignal m_reset_signal;
      CheckBox* m_security_check_box;
      CheckBox* m_side_check_box;
      CheckBox* m_size_check_box;
      CheckBox* m_ref_px_check_box;
      CheckBox* m_date_check_box;
      CheckBox* m_time_check_box;
      CheckBox* m_notional_value_check_box;
      mutable ToggledSignal m_security_signal;
      mutable ToggledSignal m_side_signal;
      mutable ToggledSignal m_size_signal;
      mutable ToggledSignal m_ref_px_signal;
      mutable ToggledSignal m_date_signal;
      mutable ToggledSignal m_time_signal;
      mutable ToggledSignal m_notional_value_signal;

      void add_check_box(CheckBox* check_box, const QString& text,
        QWidgetAction* action);
  };
}

#endif
