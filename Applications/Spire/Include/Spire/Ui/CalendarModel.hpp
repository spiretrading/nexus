#ifndef SPIRE_CALENDAR_MODEL_HPP
#define SPIRE_CALENDAR_MODEL_HPP
#include <array>
#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace Spire {

  //! Defines a calendar model for mapping a 6 week, 7 day calendar index
  //! to a date and vice-versa.
  class CalendarModel {
    public:

      CalendarModel() = default;

      //! Returns the date at the given week and day, with the week and day
      //! being indexed at 0.
      /*
        \param week The week of the month to query, from 0 to 5.
        \param day The day of the week to query, from 0 to 6.
      */
      boost::gregorian::date get_date(int week, int day);

      //! Returns the index of the day and week, respectively, of the given
      //! date. Indexes are 0-indexed.
      /*
        \param date The date to get the day and week index of.
      */
      std::tuple<int, int> get_pos(boost::gregorian::date date);

      //! Sets the month that the dates and indexes are calculated relative to.
      /*
        \param month The month to calculate dates relative to, from 1 to 12.
        \param year The year to calculate dates relative to.
      */
      void set_month(int month, int year);

    private:
      boost::gregorian::date m_reference_date;
      std::array<boost::gregorian::date, 42> m_dates;
  };
}

#endif
