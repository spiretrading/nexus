#ifndef SPIRE_CURRENCY_COMBO_BOX_HPP
#define SPIRE_CURRENCY_COMBO_BOX_HPP
#include <QLineEdit>
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Represents a combo box for selecting Currencies.
  class CurrencyComboBox : public QLineEdit {
    public:

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

    private:
      StaticDropDownMenu* m_menu;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
