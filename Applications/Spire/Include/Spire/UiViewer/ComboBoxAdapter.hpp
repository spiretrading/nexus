#ifndef SPIRE_COMBO_BOX_ADAPTER_HPP
#define SPIRE_COMBO_BOX_ADAPTER_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Provides an adapter for testing a combo box.
  class ComboBoxAdapter : public QWidget {
    public:

      //! Signal type for combo box selections.
      /*!
        \param value The selected value.
      */
      using SelectedSignal = Signal<void (const QVariant& value)>;

      //! Constructs a ComboBoxAdapter.
      /*!
        \param parent The parent widget.
      */
      explicit ComboBoxAdapter(QWidget* parent = nullptr);

      //! Connects a slot to the value selection signal.
      virtual boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const = 0;
  };
}

#endif
