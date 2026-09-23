#ifndef TMX_IP_FIELDS_HPP
#define TMX_IP_FIELDS_HPP
#include <cstdint>

namespace Nexus {

  /** STAMP field identifiers used by TMX IP CDF, CBBO and CLS. */
  namespace TmxIpFields {
    inline constexpr auto BUSINESS_ACTION = std::uint16_t(5);
    inline constexpr auto BUSINESS_CLASS = std::uint16_t(6);
    inline constexpr auto CFOD_ORDER_NUMBER = std::uint16_t(11);
    inline constexpr auto LAST_SEQUENCE_RECEIVED = std::uint16_t(15);
    inline constexpr auto CONFIRMATION_TYPE = std::uint16_t(16);
    inline constexpr auto DEST_ADDRESS = std::uint16_t(17);
    inline constexpr auto MINIMUM_FILL_VOLUME = std::uint16_t(31);
    inline constexpr auto ORDER_NUMBER = std::uint16_t(40);
    inline constexpr auto PRICE = std::uint16_t(41);
    inline constexpr auto MGF_VOLUME = std::uint16_t(49);
    inline constexpr auto SEQUENCE_NUMBER = std::uint16_t(50);
    inline constexpr auto SETTLEMENT_TERMS = std::uint16_t(53);
    inline constexpr auto SOURCE_ADDRESS = std::uint16_t(54);
    inline constexpr auto SYMBOL = std::uint16_t(55);
    inline constexpr auto TIMESTAMP = std::uint16_t(56);
    inline constexpr auto TRADING_SYS_TIMESTAMP = std::uint16_t(57);
    inline constexpr auto CURRENCY = std::uint16_t(58);
    inline constexpr auto VOLUME = std::uint16_t(64);
    inline constexpr auto VERSION_NUMBER = std::uint16_t(65);
    inline constexpr auto PRIORITY_VOLUME = std::uint16_t(68);
    inline constexpr auto BROKER_NUMBER = std::uint16_t(70);
    inline constexpr auto LOTS_OF = std::uint16_t(74);
    inline constexpr auto EXTENDED_HOURS = std::uint16_t(76);
    inline constexpr auto STOCK_HALT_DATE = std::uint16_t(80);
    inline constexpr auto RETRANS = std::uint16_t(97);
    inline constexpr auto PRODUCT_TYPE = std::uint16_t(105);
    inline constexpr auto ACCEPT_ANONYMOUS = std::uint16_t(110);
    inline constexpr auto NUMBER_OF_MESSAGES = std::uint16_t(111);
    inline constexpr auto TOTAL_NUM_MESSAGES = std::uint16_t(112);
    inline constexpr auto LAST_MESSAGE = std::uint16_t(113);
    inline constexpr auto LAST_SALE = std::uint16_t(114);
    inline constexpr auto BOARD_LOT = std::uint16_t(115);
    inline constexpr auto EQUITY_STATUS = std::uint16_t(117);
    inline constexpr auto FACE_VALUE = std::uint16_t(119);
    inline constexpr auto OPENING_TIME = std::uint16_t(120);
    inline constexpr auto RETRANS_ID = std::uint16_t(147);
    inline constexpr auto DISPLAY_VOLUME = std::uint16_t(150);
    inline constexpr auto MARKET_STATE = std::uint16_t(159);
    inline constexpr auto MESSAGE_TEXT = std::uint16_t(160);
    inline constexpr auto STOCK_STATE = std::uint16_t(161);
    inline constexpr auto SELL_PARTICIPATION = std::uint16_t(166);
    inline constexpr auto NON_RESIDENT = std::uint16_t(168);
    inline constexpr auto CUSIP = std::uint16_t(171);
    inline constexpr auto COMMENT = std::uint16_t(173);
    inline constexpr auto BEST_PRICE_GUARANTEE = std::uint16_t(175);
    inline constexpr auto SYMBOL_FULL_NAME = std::uint16_t(177);
    inline constexpr auto PRIORITY_TIMESTAMP = std::uint16_t(178);
    inline constexpr auto TRADE_CORRECTION = std::uint16_t(183);
    inline constexpr auto CALCULATED_OPENING_PRICE = std::uint16_t(191);
    inline constexpr auto ORDER_KEY = std::uint16_t(192);
    inline constexpr auto MBX_PART_NUMBER = std::uint16_t(194);
    inline constexpr auto MBX_TOTAL_PARTS = std::uint16_t(195);
    inline constexpr auto PUBLIC_PRICE = std::uint16_t(196);
    inline constexpr auto MARKET_SIDE = std::uint16_t(197);
    inline constexpr auto SPECIALIST_NAME = std::uint16_t(199);
    inline constexpr auto TRADE_NUMBER = std::uint16_t(220);
    inline constexpr auto EXCHANGE_ID = std::uint16_t(247);
    inline constexpr auto TRADE_TIMESTAMP = std::uint16_t(264);
    inline constexpr auto STOCK_GROUP = std::uint16_t(282);
    inline constexpr auto MGF_SETTING = std::uint16_t(284);
    inline constexpr auto SPECIALIST_PHONE_NUMBER = std::uint16_t(312);
    inline constexpr auto BULLETIN_INDICATOR = std::uint16_t(317);
    inline constexpr auto SUB_STOCK_STATE = std::uint16_t(361);
    inline constexpr auto CROSS_TYPE = std::uint16_t(390);
    inline constexpr auto TRADE_THROUGH_EXEMPT = std::uint16_t(392);
    inline constexpr auto BLIND_OFFSET_ACCEPTED = std::uint16_t(490);
    inline constexpr auto CALCULATED_CLOSING_PRICE = std::uint16_t(491);
    inline constexpr auto IMBALANCE_SIDE = std::uint16_t(492);
    inline constexpr auto IMBALANCE_VOLUME = std::uint16_t(493);
    inline constexpr auto MOC = std::uint16_t(494);
    inline constexpr auto MOC_VWAP = std::uint16_t(495);
    inline constexpr auto MOC_ELIGIBLE = std::uint16_t(496);
    inline constexpr auto CDF_PUB_TIMESTAMP = std::uint16_t(501);
    inline constexpr auto CDF_RCV_TIMESTAMP = std::uint16_t(502);
    inline constexpr auto BYPASS = std::uint16_t(503);
    inline constexpr auto ORIG_TRADE_ID = std::uint16_t(506);
    inline constexpr auto CDF_ID = std::uint16_t(513);
    inline constexpr auto CDF_OUTBOUND_TIMESTAMP = std::uint16_t(514);
    inline constexpr auto CDF_INBOUND_TIMESTAMP = std::uint16_t(515);
    inline constexpr auto SHORT_EXEMPT_ELIGIBLE = std::uint16_t(520);
    inline constexpr auto EXPIRY_DATE = std::uint16_t(521);
    inline constexpr auto COUPON_FREQUENCY = std::uint16_t(522);
    inline constexpr auto DIVIDEND_FREQUENCY = std::uint16_t(523);
    inline constexpr auto LISTING_MARKET = std::uint16_t(554);
    inline constexpr auto OPENING_IMBALANCE_SIDE = std::uint16_t(572);
    inline constexpr auto OPENING_IMBALANCE_VOLUME = std::uint16_t(573);
    inline constexpr auto OPENING_AUCTION = std::uint16_t(574);
    inline constexpr auto OPENING_PAIRED_VOLUME = std::uint16_t(578);
    inline constexpr auto TOTAL_NUM_OPEN_ORDERS = std::uint16_t(581);
    inline constexpr auto TOTAL_NUM_STOCK_GROUPS = std::uint16_t(582);
    inline constexpr auto TOTAL_NUM_SYMBOLS = std::uint16_t(583);
    inline constexpr auto TRADING_TIER_ID = std::uint16_t(584);
    inline constexpr auto ACCEPT_UNDISPLAYED = std::uint16_t(605);
    inline constexpr auto IS_DARK = std::uint16_t(617);
    inline constexpr auto IMBALANCE_REFERENCE_PRICE = std::uint16_t(631);
    inline constexpr auto MIN_PO_QTY = std::uint16_t(632);
    inline constexpr auto BOOK_TYPE = std::uint16_t(636);
    inline constexpr auto LIQUIDITY_TIER = std::uint16_t(637);
    inline constexpr auto PRIORITY_STATUS = std::uint16_t(639);
    inline constexpr auto PREVIOUS_PRICE = std::uint16_t(642);
    inline constexpr auto THEORETICAL_OPENING_VOLUME = std::uint16_t(654);
    inline constexpr auto TEST_SYMBOL = std::uint16_t(665);
    inline constexpr auto SECONDARY_SPECIALIST_NAME = std::uint16_t(674);
    inline constexpr auto SECONDARY_SPECIALIST_PHONE_NUMBER =
      std::uint16_t(675);
    inline constexpr auto IS_MID_ONLY = std::uint16_t(684);
    inline constexpr auto CONDITIONAL = std::uint16_t(688);
    inline constexpr auto M_ELO = std::uint16_t(689);
    inline constexpr auto LISTING_TIER = std::uint16_t(690);
    inline constexpr auto MARKET_ORDER_IMBALANCE_VOLUME = std::uint16_t(691);
    inline constexpr auto MARKET_ORDER_IMBALANCE_SIDE = std::uint16_t(692);
    inline constexpr auto NEAR_INDICATIVE_CLOSING_PRICE = std::uint16_t(693);
    inline constexpr auto FAR_INDICATIVE_CLOSING_PRICE = std::uint16_t(694);
    inline constexpr auto PRICE_VARIATION = std::uint16_t(695);
    inline constexpr auto PAIRED_VOLUME = std::uint16_t(698);
    inline constexpr auto PURESTREAM = std::uint16_t(703);
  }
}

#endif
