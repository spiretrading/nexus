#ifndef SPIRE_TIMERANGEINPUTWIDGET_HPP
#define SPIRE_TIMERANGEINPUTWIDGET_HPP
#include <Beam/Queries/Range.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/signal.hpp>
#include <QWidget>
#include "Spire/UI/PersistentWindow.hpp"

class QAbstractButton;
class Ui_TimeRangeInputWidget;

namespace Spire {

  /*! \struct TimeRangeParameter
      \brief Stores the types of time range parameters that can be queried.
   */
  struct TimeRangeParameter {

    //! Specifies the time parameter as an offset from the present.
    boost::optional<boost::posix_time::time_duration> m_offset;

    //! Specifies the time parameter as a time of day.
    boost::optional<boost::posix_time::time_duration> m_timeOfDay;

    //! Specifies the time parameter as a special query value.
    boost::optional<Beam::Range::Point> m_specialValue;
  };

  /*! \class TimeRangeInputWidget
      \brief Displays an input widget for a time range's parameters.
   */
  class TimeRangeInputWidget : public QWidget, public UI::PersistentWindow {
    public:

      //! Signals a change to the time range parameters.
      /*!
        \param startTime The new start time.
        \param endTime The new end time.
      */
      typedef boost::signals2::signal<void (const TimeRangeParameter& startTime,
        const TimeRangeParameter& endTime)> TimeRangeUpdatedSignal;

      //! Constructs a TimeRangeInputWidget.
      /*!
        \param parent The parent widget.
      */
      TimeRangeInputWidget(QWidget* parent = nullptr);

      virtual ~TimeRangeInputWidget();

      //! Returns the start time.
      TimeRangeParameter GetStartTime() const;

      //! Returns the end time.
      TimeRangeParameter GetEndTime() const;

      //! Sets the time range.
      /*!
        \param startTime The start time.
        \param endTime The end time.
      */
      void SetTimeRange(const TimeRangeParameter& startTime,
        const TimeRangeParameter& endTime);

      //! Connects a slot to the TimeRangeUpdatedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the specified signal.
      */
      boost::signals2::connection ConnectTimeRangeUpdatedSignal(
        const TimeRangeUpdatedSignal::slot_function_type& slot) const;

      virtual std::unique_ptr<UI::WindowSettings> GetWindowSettings() const;

    private:
      std::unique_ptr<Ui_TimeRangeInputWidget> m_ui;
      mutable TimeRangeUpdatedSignal m_updateSignal;

      void UpdateEnabledWidgets();
      void OnStartTimeButtonClicked(QAbstractButton* button);
      void OnStartTimeOffsetValueChanged(int value);
      void OnStartTimeOffsetUnitsIndexChanged(int index);
      void OnStartTimeOfDayChanged(const QTime& value);
      void OnEndTimeButtonClicked(QAbstractButton* button);
      void OnEndTimeOffsetValueChanged(int value);
      void OnEndTimeOffsetUnitsIndexChanged(int index);
      void OnEndTimeOfDayChanged(const QTime& value);
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::TimeRangeParameter> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::TimeRangeParameter& value,
        unsigned int version) const {
      shuttle.shuttle("offset", value.m_offset);
      shuttle.shuttle("time_of_day", value.m_timeOfDay);
      shuttle.shuttle("special_value", value.m_specialValue);
    }
  };
}

#endif
