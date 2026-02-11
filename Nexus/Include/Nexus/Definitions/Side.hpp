#ifndef NEXUS_DEFINITIONS_SIDE_HPP
#define NEXUS_DEFINITIONS_SIDE_HPP
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Collections/EnumSet.hpp>
#include "Nexus/Definitions/Quantity.hpp"

namespace Nexus {

  /** Represents a bid or an ask. */
  BEAM_ENUM(Side,

    /** Ask side. */
    ASK,

    /** Bid side. */
    BID);

  /** Stores a bitset of Sides. */
  using SideSet = Beam::EnumSet<Side>;

  /**
   * Returns a value based on a Side.
   * @param side The Side to pick the value from.
   * @param ask The value to return if <i>side</i> is an ASK.
   * @param bid The value to return if <i>side</i> is a BID.
   */
  template<typename T>
  T& pick(Side side, T& ask, T& bid) {
    if(side == Side::ASK) {
      return ask;
    }
    return bid;
  }

  /**
   * Returns a value based on a Side.
   * @param side The Side to pick the value from.
   * @param ask The value to return if <i>side</i> is an ASK.
   * @param bid The value to return if <i>side</i> is a BID.
   */
  template<typename T>
  const T& pick(Side side, const T& ask, const T& bid) {
    if(side == Side::ASK) {
      return ask;
    }
    return bid;
  }

  /** Returns the direction of the Side as a magnitude. */
  inline int get_direction(Side value) {
    if(value == Side::ASK) {
      return -1;
    } else if(value == Side::BID) {
      return 1;
    }
    return 0;
  }

  /** Returns the Side of a position. */
  inline Side get_side(Quantity quantity) {
    if(quantity > 0) {
      return Side::BID;
    } else if(quantity < 0) {
      return Side::ASK;
    }
    return Side::NONE;
  }

  /** Returns the opposite of a Side. */
  inline Side get_opposite(Side value) {
    if(value == Side::BID) {
      return Side::ASK;
    } else if(value == Side::ASK) {
      return Side::BID;
    }
    return Side::NONE;
  }

  /** Returns the single character representation of a Side. */
  inline char to_char(Side value) {
    if(value == Side::ASK) {
      return 'A';
    } else if(value == Side::BID) {
      return 'B';
    }
    return '?';
  }
}

#endif
