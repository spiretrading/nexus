#ifndef SPIRE_CALENDAR_MODEL_HPP
#define SPIRE_CALENDAR_MODEL_HPP
#include <array>
#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace Spire {

  class CalendarModel {
    public:

      CalendarModel() = default;

      // indexed at 0
      boost::gregorian::date get_date(int row, int column);

      std::tuple<int, int> get_pos(boost::gregorian::date date);

      void set_month(int month, int year);

    private:
      boost::gregorian::date m_reference_date;
      std::array<boost::gregorian::date, 42> m_dates;
  };
}

#endif
