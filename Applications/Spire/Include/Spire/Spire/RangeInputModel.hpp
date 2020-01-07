#ifndef SPIRE_RANGE_INPUT_MODEL_HPP
#define SPIRE_RANGE_INPUT_MODEL_HPP
#include <vector>
#include <QVariant>

namespace Spire {

  //! Represents a model for a range input widget.
  template <typename T>
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

      //! Constructs a RangeInputModel from the provided data.
      /*
        \param data The data used to determine minimum and maximum values
                    and create histograms.
      */
      RangeInputModel(const std::vector<QVariant>& data)
          : m_data(data) {
        if(!m_data.empty()) {
          m_minimum_value = data.front();
          m_maximum_value = data.front();
          for(auto& value : m_data) {
            m_minimum_value = min(value, m_minimum_value);
            m_maximum_value = max(value, m_maximum_value);
          }
        }
      }

      //! Returns the smallest value among the data.
      const QVariant& get_minimum_value() const {
        return m_minimum_value;
      }

      //! Returns the largest value among the data.
      const QVariant& get_maximum_value() const {
        return m_maximum_value;
      }

      //! Returns a Histogram constructed from the data.
      /*
        \param bin_count The bin count for the histogram.
      */
      Histogram make_histogram(int bin_count) {
        if(bin_count < 1) {
          return {{}, 0};
        }
        auto histogram = std::vector<int>(bin_count, 0);
        auto highest_frequency = 0;
        for(auto& value : m_data) {
          auto index =
            static_cast<double>(value.value - m_minimum_value) /
            static_cast<double>(m_maximum_value - m_minimum_value) *
            static_cast<double>(bin_count - 1);
          histogram[static_cast<int>(index)]++;
        }
        return {histogram, highest_frequency};
      }

    private:
      std::vector<QVariant> m_data;
      QVariant m_minimum_value;
      QVariant m_maximum_value;
  };
}

#endif
