#ifndef SPIRE_BBO_INDICATOR_HPP
#define SPIRE_BBO_INDICATOR_HPP
#include <QString>

namespace Nexus {
  struct BboQuote;
  struct TimeAndSale;
}

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

  /**
   * Returns the BboIndicator classifying a time and sale against the BBO
   * quote in effect when it was reported.
   * @param bbo The BBO quote in effect.
   * @param time_and_sale The time and sale to classify.
   */
  BboIndicator get_bbo_indicator(
    const Nexus::BboQuote& bbo, const Nexus::TimeAndSale& time_and_sale);

  /** Returns the text representation of a BboIndicator. */
  const QString& to_text(BboIndicator indicator);
}

#endif
