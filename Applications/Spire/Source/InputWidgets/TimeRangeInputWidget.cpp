#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include <Beam/Queries/Sequence.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Spire/InputWidgets/TimeRangeInputWidgetWindowSettings.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "ui_TimeRangeInputWidget.h"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  const int HOURS_INDEX = 0;
  const int MINUTES_INDEX = 1;

  time_duration GetOffset(const QSpinBox* inputSpinBox,
      const QComboBox* unitsComboBox) {
    time_duration offset;
    if(unitsComboBox->currentIndex() == HOURS_INDEX) {
      offset = hours(inputSpinBox->value());
    } else {
      offset = minutes(inputSpinBox->value());
    }
    return offset;
  }
}

TimeRangeInputWidget::TimeRangeInputWidget(QWidget* parent)
    : QWidget(parent),
      m_ui(std::make_unique<Ui_TimeRangeInputWidget>()) {
  m_ui->setupUi(this);
  connect(m_ui->m_startTimeButtonGroup,
    static_cast<void (QButtonGroup::*)(QAbstractButton*)>(
    &QButtonGroup::buttonClicked), this,
    &TimeRangeInputWidget::OnStartTimeButtonClicked);
  connect(m_ui->m_startTimeInputSpinBox, static_cast<void (QSpinBox::*)(int)>(
    &QSpinBox::valueChanged), this,
    &TimeRangeInputWidget::OnStartTimeOffsetValueChanged);
  connect(m_ui->m_startTimeUnitsComboBox, static_cast<void (QComboBox::*)(int)>(
    &QComboBox::currentIndexChanged), this,
    &TimeRangeInputWidget::OnStartTimeOffsetUnitsIndexChanged);
  connect(m_ui->m_startTimeOfDayInput, &QTimeEdit::timeChanged, this,
    &TimeRangeInputWidget::OnStartTimeOfDayChanged);
  connect(m_ui->m_endTimeButtonGroup,
    static_cast<void (QButtonGroup::*)(QAbstractButton*)>(
    &QButtonGroup::buttonClicked), this,
    &TimeRangeInputWidget::OnEndTimeButtonClicked);
  connect(m_ui->m_endTimeInputSpinBox, static_cast<void (QSpinBox::*)(int)>(
    &QSpinBox::valueChanged), this,
    &TimeRangeInputWidget::OnEndTimeOffsetValueChanged);
  connect(m_ui->m_endTimeUnitsComboBox, static_cast<void (QComboBox::*)(int)>(
    &QComboBox::currentIndexChanged), this,
    &TimeRangeInputWidget::OnEndTimeOffsetUnitsIndexChanged);
  connect(m_ui->m_endTimeOfDayInput, &QTimeEdit::timeChanged, this,
    &TimeRangeInputWidget::OnEndTimeOfDayChanged);
}

TimeRangeInputWidget::~TimeRangeInputWidget() {}

TimeRangeParameter TimeRangeInputWidget::GetStartTime() const {
  TimeRangeParameter timeParameter;
  if(m_ui->m_startTimeOffsetButton->isChecked()) {
    timeParameter.m_offset = GetOffset(m_ui->m_startTimeInputSpinBox,
      m_ui->m_startTimeUnitsComboBox);
  } else if(m_ui->m_startTimeOfDayButton->isChecked()) {
    timeParameter.m_timeOfDay = ToPosixTimeDuration(
      m_ui->m_startTimeOfDayInput->dateTime().toUTC().time());
  } else {
    timeParameter.m_specialValue = Beam::Sequence::PRESENT;
  }
  return timeParameter;
}

TimeRangeParameter TimeRangeInputWidget::GetEndTime() const {
  TimeRangeParameter timeParameter;
  if(m_ui->m_endTimeOffsetButton->isChecked()) {
    timeParameter.m_offset = GetOffset(m_ui->m_endTimeInputSpinBox,
      m_ui->m_endTimeUnitsComboBox);
  } else if(m_ui->m_endTimeOfDayButton->isChecked()) {
    timeParameter.m_timeOfDay = ToPosixTimeDuration(
      m_ui->m_endTimeOfDayInput->dateTime().toUTC().time());
  } else {
    timeParameter.m_specialValue = ptime(pos_infin);
  }
  return timeParameter;
}

void TimeRangeInputWidget::SetTimeRange(const TimeRangeParameter& startTime,
    const TimeRangeParameter& endTime) {
  if(startTime.m_offset.is_initialized()) {
    m_ui->m_startTimeOffsetButton->setChecked(true);
    time_duration offset = *startTime.m_offset;
    if(offset.minutes() % 60 == 0) {
      m_ui->m_startTimeInputSpinBox->setValue(
        static_cast<int>(offset.hours()));
      m_ui->m_startTimeUnitsComboBox->setCurrentIndex(HOURS_INDEX);
    } else {
      m_ui->m_startTimeInputSpinBox->setValue(
        static_cast<int>(offset.minutes()));
      m_ui->m_startTimeUnitsComboBox->setCurrentIndex(MINUTES_INDEX);
    }
  } else if(startTime.m_timeOfDay.is_initialized()) {
    m_ui->m_startTimeOfDayButton->setChecked(true);
    ptime timeOfDay(gregorian::day_clock::universal_day(),
      *startTime.m_timeOfDay);
    ptime localTime = to_local_time(timeOfDay);
    m_ui->m_startTimeOfDayInput->setDateTime(ToQDateTime(localTime));
  } else {
    m_ui->m_presentButton->setChecked(true);
  }
  if(endTime.m_offset.is_initialized()) {
    m_ui->m_endTimeOffsetButton->setChecked(true);
    time_duration offset = *endTime.m_offset;
    if(offset.minutes() % 60 == 0) {
      m_ui->m_endTimeInputSpinBox->setValue(
        static_cast<int>(offset.hours()));
      m_ui->m_endTimeUnitsComboBox->setCurrentIndex(HOURS_INDEX);
    } else {
      m_ui->m_endTimeInputSpinBox->setValue(
        static_cast<int>(offset.minutes()));
      m_ui->m_endTimeUnitsComboBox->setCurrentIndex(MINUTES_INDEX);
    }
  } else if(endTime.m_timeOfDay.is_initialized()) {
    m_ui->m_endTimeOfDayButton->setChecked(true);
    ptime timeOfDay(gregorian::day_clock::universal_day(),
      *endTime.m_timeOfDay);
    ptime localTime = to_local_time(timeOfDay);
    m_ui->m_endTimeOfDayInput->setDateTime(ToQDateTime(localTime));
  } else {
    m_ui->m_realTimeButton->setChecked(true);
  }
  UpdateEnabledWidgets();
}

connection TimeRangeInputWidget::ConnectTimeRangeUpdatedSignal(
    const TimeRangeUpdatedSignal::slot_function_type& slot) const {
  return m_updateSignal.connect(slot);
}

unique_ptr<WindowSettings> TimeRangeInputWidget::GetWindowSettings() const {
  return std::make_unique<TimeRangeInputWidgetWindowSettings>(*this);
}

void TimeRangeInputWidget::UpdateEnabledWidgets() {
  bool startTimeOffsetChecked = m_ui->m_startTimeOffsetButton->isChecked();
  m_ui->m_startTimeInputSpinBox->setEnabled(startTimeOffsetChecked);
  m_ui->m_startTimeUnitsComboBox->setEnabled(startTimeOffsetChecked);
  m_ui->m_startTimeSpecifierLabel->setEnabled(startTimeOffsetChecked);
  m_ui->m_startTimeOfDayInput->setEnabled(
    m_ui->m_startTimeOfDayButton->isChecked());
  bool endTimeOffsetChecked = m_ui->m_endTimeOffsetButton->isChecked();
  m_ui->m_endTimeInputSpinBox->setEnabled(endTimeOffsetChecked);
  m_ui->m_endTimeUnitsComboBox->setEnabled(endTimeOffsetChecked);
  m_ui->m_endTimeSpecifierLabel->setEnabled(endTimeOffsetChecked);
  m_ui->m_endTimeOfDayInput->setEnabled(
    m_ui->m_endTimeOfDayButton->isChecked());
}

void TimeRangeInputWidget::OnStartTimeButtonClicked(QAbstractButton* button) {
  m_updateSignal(GetStartTime(), GetEndTime());
  UpdateEnabledWidgets();
}

void TimeRangeInputWidget::OnStartTimeOffsetValueChanged(int value) {
  m_updateSignal(GetStartTime(), GetEndTime());
}

void TimeRangeInputWidget::OnStartTimeOffsetUnitsIndexChanged(int index) {
  m_updateSignal(GetStartTime(), GetEndTime());
}

void TimeRangeInputWidget::OnStartTimeOfDayChanged(const QTime& value) {
  m_updateSignal(GetStartTime(), GetEndTime());
}

void TimeRangeInputWidget::OnEndTimeButtonClicked(QAbstractButton* button) {
  m_updateSignal(GetStartTime(), GetEndTime());
  UpdateEnabledWidgets();
}

void TimeRangeInputWidget::OnEndTimeOffsetValueChanged(int value) {
  m_updateSignal(GetStartTime(), GetEndTime());
}

void TimeRangeInputWidget::OnEndTimeOffsetUnitsIndexChanged(int index) {
  m_updateSignal(GetStartTime(), GetEndTime());
}

void TimeRangeInputWidget::OnEndTimeOfDayChanged(const QTime& value) {
  m_updateSignal(GetStartTime(), GetEndTime());
}
