#ifndef SPIRE_CALENDAR_DATE_PICKER_HPP
#define SPIRE_CALENDAR_DATE_PICKER_HPP
#include <boost/date_time/gregorian/greg_date.hpp>
#include <QWidget>
#include "Spire/Spire/ScalarValueModel.hpp"

namespace Spire {

  using OptionalDateModel =
    ScalarValueModel<boost::optional<boost::gregorian::date>>;

  using LocalOptionalDateModel =
    LocalScalarValueModel<boost::optional<boost::gregorian::date>>;

  class CalendarDatePicker : public QWidget {
    public:

      CalendarDatePicker(QWidget* parent = nullptr);

      CalendarDatePicker(boost::gregorian::date current,
        QWidget* parent = nullptr);

      CalendarDatePicker(std::shared_ptr<OptionalDateModel> model,
        QWidget* parent = nullptr);

      const std::shared_ptr<OptionalDateModel>& get_model() const;

    private:
      class MonthSelector;
      std::shared_ptr<OptionalDateModel> m_model;
      MonthSelector* m_month_selector;
      ListView* m_calendar_view;
      std::shared_ptr<ArrayListModel> m_calendar_model;

      void populate_calendar(const std::function<
        void (int index, boost::gregorian::date day)> assign);
      void update_calendar_model();
      void on_current(const boost::optional<boost::gregorian::date>& day);
  };
}

#endif
