#ifndef SPIRE_LOCAL_RANGE_INPUT_MODEL_HPP
#define SPIRE_LOCAL_RANGE_INPUT_MODEL_HPP
#include "Spire/Spire/RangeInputModel.hpp"

namespace Spire {

  //! Implements a RangeInputModel in memory.
  class LocalRangeInputModel : public RangeInputModel {
    public:

      //! Constructs a LocalRangeInputModel from provided data, with the
      //! minimum and maximum values inferred from the data.
      /*
        \param data The data for the model.
      */
      LocalRangeInputModel(std::vector<ChartValue> data);

      ChartValue get_minimum_value() const override;

      ChartValue get_maximum_value() const override;

      Histogram make_histogram(int bin_count) const override;

    private:
      std::vector<ChartValue> m_data;
      ChartValue m_minimum_value;
      ChartValue m_maximum_value;
  };
}

#endif
