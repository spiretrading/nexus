#ifndef SPIRE_CURRENCY_LIST_ITEM_HPP
#define SPIRE_CURRENCY_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/Currency.hpp"

namespace Spire {

  /** Represents a currency list item. */
  class CurrencyListItem : public QWidget {
    public:

      /**
       * Constructs a CurrencyListItem.
       * @param entry The currency entry to display.
       * @param parent The parent widget.
       */
      explicit CurrencyListItem(
        Nexus::CurrencyDatabase::Entry entry, QWidget* parent = nullptr);

      /** Returns the currency entry. */
      const Nexus::CurrencyDatabase::Entry& get_entry() const;

    private:
      Nexus::CurrencyDatabase::Entry m_entry;
  };
}

#endif
