#ifndef SPIRE_TIME_IN_FORCE_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_TIME_IN_FORCE_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with a TimeInForceComboBox being
  //! tested.
  class TimeInForceComboBoxTestWidget : public ComboBoxAdapter {
    public:

      //! Constructs a TimeInForceComboBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit TimeInForceComboBoxTestWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const override;

    private:
      mutable SelectedSignal m_selected_signal;
  };
}

#endif
