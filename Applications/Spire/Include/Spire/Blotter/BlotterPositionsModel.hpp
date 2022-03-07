#ifndef SPIRE_BLOTTER_POSITIONS_MODEL_HPP
#define SPIRE_BLOTTER_POSITIONS_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"

namespace Spire {

  /** Defines the interface representing a blotter's positions. */
  class BlotterPositionsModel {
    public:
      virtual ~BlotterPositionsModel() = default;

    protected:

      /** Constructs a BlotterPositionsModel. */
      BlotterPositionsModel() = default;

    private:
      BlotterPositionsModel(const BlotterPositionsModel&) = delete;
      BlotterPositionsModel& operator =(const BlotterPositionsModel&) = delete;
  };
}

#endif
