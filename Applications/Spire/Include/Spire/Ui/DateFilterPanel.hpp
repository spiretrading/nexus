#ifndef SPIRE_DATE_FILTER_PANEL_HPP
#define SPIRE_DATE_FILTER_PANEL_HPP
#include <variant>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <QWidget>
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {
  class FilterPanel;

  /**
   * A panel for filtering dates by either an absolute date range or a relative
   * offset from the current date.
   */
  class DateFilterPanel : public QWidget {
    public:

      /** The date unit for the offset. */
      enum class DateUnit {

        /** Day. */
        DAY,

        /** Week. */
        WEEK,

        /** Month. */
        MONTH,

        /** Year. */
        YEAR
      };

      /** Represents an absolute date range. */
      struct AbsoluteDateRange {

        /** The range's start date. */
        boost::gregorian::date m_start;

        /** The range's end date. */
        boost::gregorian::date m_end;
      };

      /**
       * Represents a date range relative to the current date,
       * expressed in units of dates prior to today.
       */
      struct RelativeDateRange {

        /** The date unit of the offset. */
        DateUnit m_unit;

        /** The number of units to offset by. */
        int m_value;
      };

      /** Represents a date range. */
      using DateRange = std::variant<AbsoluteDateRange, RelativeDateRange>;

      /** A ValueModel over a DateRange. */
      using DateRangeModel = ValueModel<DateRange>;

      /**
       * Constructs a DateFilterPanel.
       * @param current The current date range model.
       * @param parent The parent widget.
       */
      explicit DateFilterPanel(std::shared_ptr<DateRangeModel> current,
        QWidget* parent = nullptr);

      /** Returns the current model. */
      const std::shared_ptr<DateRangeModel>& get_current() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      enum class Mode {
        OFFSET,
        RANGE,
      };
      class DateRangeModeButtonGroup;
      struct DateRangeComposerModel;
      std::unique_ptr<DateRangeComposerModel> m_model;
      DateRange m_default_date_range;
      QWidget* m_offset_value_box;
      std::unique_ptr<DateRangeModeButtonGroup> m_range_mode_button_group;

      void on_reset();
  };
}

#endif
