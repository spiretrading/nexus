#ifndef SPIRE_BLOTTER_PROFIT_AND_LOSS_MODEL_HPP
#define SPIRE_BLOTTER_PROFIT_AND_LOSS_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"

namespace Spire {

  /** Defines the interface representing a blotter's profit and loss. */
  class BlotterProfitAndLossModel {
    public:
      virtual ~BlotterProfitAndLossModel() = default;

    protected:

      /** Constructs a BlotterProfitAndLossModel. */
      BlotterProfitAndLossModel() = default;

    private:
      BlotterProfitAndLossModel(const BlotterProfitAndLossModel&) = delete;
      BlotterProfitAndLossModel& operator =(const BlotterProfitAndLossModel&) =
        delete;
  };
}

#endif
