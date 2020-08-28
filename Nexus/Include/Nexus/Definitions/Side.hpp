#ifndef NEXUS_SIDE_HPP
#define NEXUS_SIDE_HPP
#include <cassert>
#include <ostream>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Collections/EnumSet.hpp>
#include "Nexus/Definitions/Definitions.hpp"
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
   * @param askValue The value to return if <i>side</i> is an ASK.
   * @param bidValue The value to return if <i>side</i> is a BID.
   */
  template<typename T>
  T& Pick(Side side, T& askValue, T& bidValue) {
    if(side == Side::ASK) {
      return askValue;
    }
    return bidValue;
  }

  /**
   * Returns a value based on a Side.
   * @param side The Side to pick the value from.
   * @param askValue The value to return if <i>side</i> is an ASK.
   * @param bidValue The value to return if <i>side</i> is a BID.
   */
  template<typename T>
  const T& Pick(Side side, const T& askValue, const T& bidValue) {
    if(side == Side::ASK) {
      return askValue;
    }
    return bidValue;
  }

  /** Returns the direction of the Side as a magnitude. */
  inline int GetDirection(Side value) {
    if(value == Side::ASK) {
      return -1;
    } else if(value == Side::BID) {
      return 1;
    }
    return 0;
  }

  /** Returns the Side of a position. */
  inline Side GetSide(Quantity quantity) {
    if(quantity > 0) {
      return Side::BID;
    } else if(quantity < 0) {
      return Side::ASK;
    }
    return Side::NONE;
  }

  /** Returns the opposite of a Side. */
  inline Side GetOpposite(Side value) {
    if(value == Side::BID) {
      return Side::ASK;
    } else if(value == Side::ASK) {
      return Side::BID;
    }
    return Side::NONE;
  }

  /** Returns the single character representation of a Side. */
  inline char ToChar(Side value) {
    if(value == Side::ASK) {
      return 'A';
    } else if(value == Side::BID) {
      return 'B';
    }
    return '?';
  }

  inline std::ostream& operator <<(std::ostream& out, Side side) {
    if(side == Side::ASK) {
      return out << "ASK";
    } else if(side == Side::BID) {
      return out << "BID";
    }
    return out << "NONE";
  }
}

#endif
