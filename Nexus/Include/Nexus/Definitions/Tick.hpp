#ifndef NEXUS_TICK_HPP
#define NEXUS_TICK_HPP
#include <Beam/Utilities/Enumerator.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {
namespace Details {

  /*! \struct TickDirectionDefinition
      \brief The direction of a Security's tick.
    */
  struct TickDirectionDefinition {
    enum Type {

      //! Neutral tick.
      NEUTRAL,

      //! Up tick.
      UP,

      //! Down tick.
      DOWN
    };
  };
}

  typedef Beam::Enumerator<Details::TickDirectionDefinition> TickDirection;

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
