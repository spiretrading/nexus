#include "Spire/Charting/ChartIntervalComboBox.hpp"
#include <regex>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <QLineEdit>
#include "Spire/Charting/ChartIntervalComboBoxWindowSettings.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "ui_ChartIntervalComboBox.h"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  bool IsDivisible(time_duration a, time_duration b) {
    return a.ticks() % b.ticks() == 0;
  }
}

ChartIntervalComboBox::IntervalEntry::IntervalEntry(time_duration value)
    : m_value(value) {
  if(IsDivisible(value, hours(1))) {
    m_name = lexical_cast<string>(value.hours()) + " hour";
  } else if(IsDivisible(value, minutes(1))) {
    m_name = lexical_cast<string>(value.total_seconds() / 60) + " minute";
  } else {
    m_name = lexical_cast<string>(value.total_seconds()) + " second";
  }
}

ChartIntervalComboBox::ChartIntervalComboBox(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(make_unique<Ui_ChartIntervalComboBox>()) {
  m_ui->setupUi(this);
  m_ui->m_comboBox->installEventFilter(this);
  SetInterval(DurationType::GetInstance(),
    ChartValue(time_duration(minutes(1))));
  connect(m_ui->m_comboBox,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, &ChartIntervalComboBox::OnCurrentIndexChanged);
}

ChartIntervalComboBox::~ChartIntervalComboBox() {}

const NativeType& ChartIntervalComboBox::GetType() const {
  return *m_type;
}

ChartValue ChartIntervalComboBox::GetValue() const {
  return m_value;
}

void ChartIntervalComboBox::SetInterval(std::shared_ptr<NativeType> type,
    ChartValue value) {
  if(type->GetCompatibility(DurationType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL) {
    SetInterval(value.ToTimeDuration());
  }
}

void ChartIntervalComboBox::SetInterval(time_duration value) {
  if(ChartValue(value) == m_value && m_type->GetCompatibility(
      DurationType::GetInstance()) == CanvasType::Compatibility::EQUAL) {
    return;
  }
  m_ui->m_comboBox->blockSignals(true);
  m_ui->m_comboBox->clear();
  IntervalEntry entry(value);
  m_entries = GetStandardDurationIntervals();
  auto index = -1;
  auto entryIterator = m_entries.begin();
  while(entryIterator != m_entries.end()) {
    if(entryIterator->m_value == entry.m_value) {
      index = std::distance(m_entries.begin(), entryIterator);
      m_ui->m_comboBox->addItem(tr(entryIterator->m_name.c_str()));
      ++entryIterator;
    } else if(entryIterator->m_value > entry.m_value && index == -1) {
      index = std::distance(m_entries.begin(), entryIterator);
      entryIterator = m_entries.insert(entryIterator, entry);
      m_ui->m_comboBox->addItem(tr(entry.m_name.c_str()));
      m_ui->m_comboBox->addItem(tr(entryIterator->m_name.c_str()));
    } else {
      m_ui->m_comboBox->addItem(tr(entryIterator->m_name.c_str()));
      ++entryIterator;
    }
  }
  m_value = ChartValue(value);
  m_type = DurationType::GetInstance();
  m_ui->m_comboBox->setCurrentIndex(index);
  m_ui->m_comboBox->blockSignals(false);
  m_intervalChangedSignal(m_type, m_value);
}

connection ChartIntervalComboBox::ConnectIntervalChangedSignal(
    const IntervalChangedSignal::slot_function_type& slot) const {
  return m_intervalChangedSignal.connect(slot);
}

std::unique_ptr<WindowSettings> ChartIntervalComboBox::
    GetWindowSettings() const {
  return std::make_unique<ChartIntervalComboBoxWindowSettings>(*this);
}

bool ChartIntervalComboBox::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_comboBox) {
    if(event->type() == QEvent::FocusIn) {
      m_ui->m_comboBox->setEditable(true);
      m_ui->m_comboBox->lineEdit()->selectAll();
      return true;
    } else if(event->type() == QEvent::FocusOut) {
      m_ui->m_comboBox->setEditable(false);
      return true;
    }
  }
  return QWidget::eventFilter(object, event);
}

vector<ChartIntervalComboBox::IntervalEntry>
    ChartIntervalComboBox::GetStandardDurationIntervals() const {
  vector<IntervalEntry> standardDurationIntervals;
  standardDurationIntervals.push_back(IntervalEntry(seconds(1)));
  standardDurationIntervals.push_back(IntervalEntry(seconds(2)));
  standardDurationIntervals.push_back(IntervalEntry(seconds(5)));
  standardDurationIntervals.push_back(IntervalEntry(seconds(10)));
  standardDurationIntervals.push_back(IntervalEntry(seconds(15)));
  standardDurationIntervals.push_back(IntervalEntry(seconds(30)));
  standardDurationIntervals.push_back(IntervalEntry(minutes(1)));
  standardDurationIntervals.push_back(IntervalEntry(minutes(2)));
  standardDurationIntervals.push_back(IntervalEntry(minutes(5)));
  standardDurationIntervals.push_back(IntervalEntry(minutes(10)));
  standardDurationIntervals.push_back(IntervalEntry(minutes(15)));
  standardDurationIntervals.push_back(IntervalEntry(minutes(30)));
  standardDurationIntervals.push_back(IntervalEntry(hours(1)));
  standardDurationIntervals.push_back(IntervalEntry(hours(2)));
  standardDurationIntervals.push_back(IntervalEntry(hours(4)));
  return standardDurationIntervals;
}

void ChartIntervalComboBox::OnCurrentIndexChanged(int index) {
  regex durationRegex("^\\s*(\\d{0,4})\\s*(\\w+)\\s*$");
  smatch matcher;
  auto textValue = m_ui->m_comboBox->itemText(index).toStdString();
  if(regex_match(textValue, matcher, durationRegex)) {
    int value = lexical_cast<int>(string(matcher[1]));
    string unit = to_lower_copy(string(matcher[2]));
    if(string("minutes").substr(0, unit.size()) == unit) {
      SetInterval(minutes(value));
      return;
    } else if(string("seconds").substr(0, unit.size()) == unit) {
      SetInterval(seconds(value));
      return;
    } else if(string("hours").substr(0, unit.size()) == unit) {
      SetInterval(hours(value));
      return;
    }
  }
}
