#ifndef NEXUS_FIX_TAGS_HPP
#define NEXUS_FIX_TAGS_HPP
#include <string_view>
#include "Nexus/Definitions/Tag.hpp"

namespace Nexus {

  /** FIX tag 100 - ExDestination. */
  constexpr auto EX_DESTINATION_KEY = 100;

  /** FIX tag 18 - ExecInst. */
  constexpr auto EXEC_INST_KEY = 18;

  /** FIX ExecInst value for market peg. */
  constexpr auto MARKET_PEG = std::string_view("P");

  /** FIX tag 111 - MaxFloor. */
  constexpr auto MAX_FLOOR_KEY = 111;

  /** FIX ExecInst value for mid-price peg. */
  constexpr auto MID_PRICE_PEG = std::string_view("M");

  /** FIX tag 211 - PegDifference. */
  constexpr auto PEG_DIFFERENCE_KEY = 211;

  /** FIX ExecInst value for primary peg. */
  constexpr auto PRIMARY_PEG = std::string_view("R");

  /**
   * Makes an ExDestination tag.
   * @param destination The destination identifier.
   */
  inline Tag make_ex_destination(std::string_view destination) {
    return Tag(EX_DESTINATION_KEY, std::string(destination));
  }

  /**
   * Makes an ExecInst tag.
   * @param instruction The execution instruction value.
   */
  inline Tag make_exec_inst(std::string_view instruction) {
    return Tag(EXEC_INST_KEY, std::string(instruction));
  }

  /**
   * Makes a MaxFloor tag.
   * @param quantity The max floor quantity.
   */
  inline Tag make_max_floor(Quantity quantity) {
    return Tag(MAX_FLOOR_KEY, quantity);
  }

  /**
   * Makes a PegDifference tag.
   * @param difference The peg difference.
   */
  inline Tag make_peg_difference(Money difference) {
    return Tag(PEG_DIFFERENCE_KEY, difference);
  }
}

#endif
