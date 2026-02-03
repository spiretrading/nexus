#ifndef OTC_LINK_CHANNEL_ID_HPP
#define OTC_LINK_CHANNEL_ID_HPP

namespace Nexus {

  /** OTC Link multicast channel identifiers. */
  enum class OtcLinkChannelId {

    /** OTC Link trade data (real time). */
    TRADE_REAL_TIME = 1,

    /** Reference data with CUSIP (real time). */
    REFERENCE_DATA_WITH_CUSIP_REAL_TIME = 5,

    /** Reference data without CUSIP (real time). */
    REFERENCE_DATA_WITHOUT_CUSIP_REAL_TIME = 6,

    /** Reference data with CUSIP (snapshot). */
    REFERENCE_DATA_WITH_CUSIP_SNAPSHOT = 7,

    /** Reference data without CUSIP (snapshot). */
    REFERENCE_DATA_WITHOUT_CUSIP_SNAPSHOT = 8,

    /** Quote book (real time). */
    QUOTE_BOOK_REAL_TIME = 11,

    /** Quote book (snapshot). */
    QUOTE_BOOK_SNAPSHOT = 12,

    /** Quote inside (real time). */
    QUOTE_INSIDE_REAL_TIME = 14,

    /** Quote inside (snapshot). */
    QUOTE_INSIDE_SNAPSHOT = 15,

    /** Quote reference price (real time). */
    QUOTE_REFERENCE_PRICE_REAL_TIME = 17,

    /** Quote reference price (snapshot). */
    QUOTE_REFERENCE_PRICE_SNAPSHOT = 18,

    /** Quote book with Global OTC (real time). */
    QUOTE_BOOK_WITH_GLOBAL_OTC_REAL_TIME = 19,

    /** Quote book with Global OTC (snapshot). */
    QUOTE_BOOK_WITH_GLOBAL_OTC_SNAPSHOT = 20,

    /** Quote inside with Global OTC (real time). */
    QUOTE_INSIDE_WITH_GLOBAL_OTC_REAL_TIME = 21,

    /** Quote inside with Global OTC (snapshot). */
    QUOTE_INSIDE_WITH_GLOBAL_OTC_SNAPSHOT = 22
  };
}

#endif
