#ifndef SPIRE_ORDER_TYPE_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_ORDER_TYPE_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with an OrderTypeComboBox being
  //! tested.
  class OrderTypeComboBoxTestWidget : public ComboBoxAdapter {
    public:

      //! Constructs an OrderTypeComboBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit OrderTypeComboBoxTestWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const override;

    private:
      mutable SelectedSignal m_selected_signal;
  };
}

#endif
