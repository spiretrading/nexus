#ifndef SPIRE_MARKET_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_MARKET_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with a MarketComboBox being tested.
  class MarketComboBoxTestWidget : public ComboBoxAdapter {
    public:

      //! Constructs a MarketComboBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit MarketComboBoxTestWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
  };
}

#endif
