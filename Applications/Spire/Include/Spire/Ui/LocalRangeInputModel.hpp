#ifndef SPIRE_LOCAL_RANGE_INPUT_MODEL_HPP
#define SPIRE_LOCAL_RANGE_INPUT_MODEL_HPP
#include <vector>
#include "Spire/Ui/RangeInputModel.hpp"

namespace Spire {

  //! Implements a RangeInputModel in memory.
  class LocalRangeInputModel : public RangeInputModel {
    public:

      //! Constructs a LocalRangeInputModel from provided data, with the
      //! minimum and maximum values inferred from the data.
      /*
        \param data The data for the model.
      */
      LocalRangeInputModel(std::vector<Scalar> data);

      Scalar get_minimum_value() const override;

      Scalar get_maximum_value() const override;

      Histogram make_histogram(int bin_count) const override;

    private:
      std::vector<Scalar> m_data;
      Scalar m_minimum_value;
      Scalar m_maximum_value;
  };
}

#endif
