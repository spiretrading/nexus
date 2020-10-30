#ifndef SPIRE_SIDE_COMBO_BOX_TEST_WIDGET_HPP
#define SPIRE_SIDE_COMBO_BOX_TEST_WIDGET_HPP
#include "Spire/UiViewer/ComboBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with a SideComboBox being tested.
  class SideComboBoxTestWidget : public ComboBoxAdapter {
    public:

      //! Constructs a SideComboBoxTestWidget.
      /*!
        \param parent The parent widget.
      */
      explicit SideComboBoxTestWidget(QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const override;

    private:
      mutable SelectedSignal m_selected_signal;
  };
}

#endif
