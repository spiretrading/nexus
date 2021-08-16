#ifndef SPIRE_CALENDAR_DATE_PICKER_HPP
#define SPIRE_CALENDAR_DATE_PICKER_HPP
#include <QDate>
#include <QWidget>
#include "Spire/Spire/ScalarValueModel.hpp"

namespace Spire {

  using DateModel = ScalarValueModel<boost::optional<QDate>>;

  using LocalDateModel = LocalScalarValueModel<boost::optional<QDate>>;

  class CalendarDatePicker : public QWidget {
    public:

      CalendarDatePicker(QWidget* parent = nullptr);

      CalendarDatePicker(QDate current, QWidget* parent = nullptr);

      CalendarDatePicker(std::shared_ptr<DateModel> model,
        QWidget* parent = nullptr);

      const std::shared_ptr<DateModel>& get_model() const;

    private:
      class MonthSelector;
      std::shared_ptr<DateModel> m_model;
      MonthSelector* m_month_selector;

      void on_current(const boost::optional<QDate>& date);
  };
}

#endif
