#ifndef SPIRE_RANGE_INPUT_MODEL_HPP
#define SPIRE_RANGE_INPUT_MODEL_HPP
#include "Spire/Spire/Scalar.hpp"

namespace Spire {

  //! Represents a model for a range input widget.
  class RangeInputModel {
    public:

      //! Represents a histogram for a range input widget.
      struct Histogram {
      
        //! A histogram where each element is a bin, and the value
        //! of the element is the bin's frequency.
        std::vector<int> m_histogram;

        //! The highest frequency count amoung all the histogram's bins.
        int m_highest_frequency;
      };

      virtual ~RangeInputModel() = default;

      //! Returns the smallest value among the data.
      virtual Scalar get_minimum_value() const = 0;

      //! Returns the largest value among the data.
      virtual Scalar get_maximum_value() const = 0;

      //! Returns a Histogram constructed from the data.
      /*
        \param bin_count The bin count for the histogram.
      */
      virtual Histogram make_histogram(int bin_count) const = 0;
  };
}

#endif
