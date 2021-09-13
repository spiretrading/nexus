#ifndef SPIRE_CALENDAR_DATE_PICKER_HPP
#define SPIRE_CALENDAR_DATE_PICKER_HPP
#include <boost/date_time/gregorian/greg_date.hpp>
#include <QWidget>
#include "Spire/Spire/ScalarValueModel.hpp"
#include "Spire/Styles/StateSelector.hpp"

namespace Spire {
namespace Styles {

  using Today = StateSelector<void, struct TodayTag>;

  using OutOfMonth = StateSelector<void, struct OutOfMonthTag>;
}

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

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      class MonthSelector;
      class CalendarDayLabel;
      std::shared_ptr<OptionalDateModel> m_model;
      MonthSelector* m_month_selector;
      QWidget* m_day_header;
      ListView* m_calendar_view;
      std::vector<CalendarDayLabel*> m_day_labels;
      std::shared_ptr<ArrayListModel> m_calendar_model;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_list_current_connection;

      void populate_calendar(const std::function<
        void (int index, boost::gregorian::date day)> assign);
      void update_calendar_model();
      void on_current_month(boost::gregorian::date month);
      void on_list_current(const boost::optional<int> index);
  };
}

#endif
