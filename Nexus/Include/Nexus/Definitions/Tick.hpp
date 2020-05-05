#ifndef NEXUS_TICK_HPP
#define NEXUS_TICK_HPP
#include <Beam/Collections/Enum.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \enum TickDirection
      \brief The direction of a Security's tick.
    */
  BEAM_ENUM(TickDirection,

    //! Neutral tick.
    NEUTRAL,

    //! Up tick.
    UP,

    //! Down tick.
    DOWN
  );

  //! Returns the single character representation of a TickDirection.
  inline char ToChar(TickDirection tick) {
    if(tick == TickDirection::NEUTRAL) {
      return 'N';
    } else if(tick == TickDirection::UP) {
      return 'U';
    } else if(tick == TickDirection::DOWN) {
      return 'D';
    }
    return '?';
  }
}

#endif
