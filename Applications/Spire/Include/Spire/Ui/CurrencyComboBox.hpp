#ifndef SPIRE_CURRENCY_COMBO_BOX_HPP
#define SPIRE_CURRENCY_COMBO_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting Currencies.
  class CurrencyComboBox : public QWidget {
    public:

      //! Signals that a Currency was selected.
      /*!
        \param currency The Id of the selected Currency.
      */
      using SelectedSignal = Signal<void (Nexus::CurrencyId currency)>;

      //! Constructs a CurrencyComboBox.
      /*!
        \param database The source currency database.
        \param parent The parent widget.
      */
      explicit CurrencyComboBox(const Nexus::CurrencyDatabase& database,
        QWidget* parent = nullptr);

      //! Returns the CurrencyId of the current Currency.
      Nexus::CurrencyId get_currency() const;

      //! Sets the Id of the current Currency.
      void set_currency(Nexus::CurrencyId currency);

      //! Connects a slot to the Currency selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
