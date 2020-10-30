#ifndef SPIRE_ORDER_STATUS_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_ORDER_STATUS_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with an OrderStatusComboBox being
  //! tested.
  class OrderStatusComboBoxTestWidget : public ComboBoxAdapter {
    public:

      //! Constructs an OrderStatusComboBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit OrderStatusComboBoxTestWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const override;

    private:
      mutable SelectedSignal m_selected_signal;
  };
}

#endif
