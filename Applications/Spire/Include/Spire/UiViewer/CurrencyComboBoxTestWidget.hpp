#ifndef SPIRE_CURRENCY_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_CURRENCY_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with a CurrencyComboBox being tested.
  class CurrencyComboBoxTestWidget : public ComboBoxAdapter {
    public:

      //! Constructs a CurrencyComboBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit CurrencyComboBoxTestWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const override;

    private:
      mutable SelectedSignal m_selected_signal;
  };
}

#endif
