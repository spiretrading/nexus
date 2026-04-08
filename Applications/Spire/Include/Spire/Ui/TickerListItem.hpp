#ifndef SPIRE_TICKER_LIST_ITEM_HPP
#define SPIRE_TICKER_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/TickerInfo.hpp"

namespace Spire {

  /** Represents a ticker list item in a TickerBox. */
  class TickerListItem : public QWidget {
    public:

      /**
       * Constructs a TickerListItem.
       * @param ticker_info The ticker to be displayed in the TickerListItem.
       * @param parent The parent widget.
       */
      explicit TickerListItem(
        Nexus::TickerInfo ticker_info, QWidget* parent = nullptr);

      /** Returns the ticker. */
      const Nexus::TickerInfo& get_ticker() const;

    private:
      Nexus::TickerInfo m_ticker_info;
  };
}

#endif
