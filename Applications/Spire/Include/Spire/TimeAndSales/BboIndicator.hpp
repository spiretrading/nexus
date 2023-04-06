#ifndef SPIRE_BBO_INDICATOR_HPP
#define SPIRE_BBO_INDICATOR_HPP

namespace Spire {

  /** An indicator that relates the price to the bbo. */
  enum class BboIndicator {

    /** The BBO is unknown. */
    UNKNOWN,

    /** The price is above the ask. */
    ABOVE_ASK,

    /** The price is equal to the ask. */
    AT_ASK,

    /** The price is between the bid and the ask. */
    INSIDE,

    /** The price is equal to the bid. */
    AT_BID,

    /** The price is below the bid. */
    BELOW_BID
  };

  /* The number of bbo indicators. */
  static const auto BBO_INDICATOR_COUNT = 6;
}

#endif
