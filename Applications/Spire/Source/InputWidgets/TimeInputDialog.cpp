#include "Spire/InputWidgets/TimeInputDialog.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_TimeInputDialog.h"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Spire;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 1000;

  QTime PosixTimeToQTime(const time_duration& value) {
    QTime qTime(static_cast<int>(value.hours()),
      static_cast<int>(value.minutes()), static_cast<int>(value.seconds()),
      static_cast<int>(value.fractional_seconds()) / 1000);
    return qTime;
  }
}

TimeInputDialog::TimeInputDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui{std::make_unique<Ui_TimeInputDialog>()},
      m_userProfile(userProfile.Get()) {
  m_ui->setupUi(this);
  auto currentTime = ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime());
  m_ui->m_currentTimeDisplay->setTime(PosixTimeToQTime(
    currentTime.time_of_day()));
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &TimeInputDialog::accept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &TimeInputDialog::reject);
  connect(&m_currentTimer, &QTimer::timeout, this,
    &TimeInputDialog::OnCurrentTimer);
  m_currentTimer.start(UPDATE_INTERVAL);
}

TimeInputDialog::TimeInputDialog(const time_duration& initialValue,
    Ref<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_TimeInputDialog>()),
      m_userProfile(userProfile.Get()) {
  m_ui->setupUi(this);
  auto currentTime = ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime());
  m_ui->m_currentTimeDisplay->setTime(PosixTimeToQTime(
    currentTime.time_of_day()));
  if(!initialValue.is_special() && !initialValue.is_not_a_date_time()) {
    auto localTime = ToLocalTime(initialValue);
    m_ui->m_selectedTimeInput->setTime(PosixTimeToQTime(localTime));
  }
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &TimeInputDialog::accept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &TimeInputDialog::reject);
  connect(&m_currentTimer, &QTimer::timeout, this,
    &TimeInputDialog::OnCurrentTimer);
  m_currentTimer.start(UPDATE_INTERVAL);
}

TimeInputDialog::~TimeInputDialog() {}

time_duration TimeInputDialog::GetTime() const {
  auto timeInput = m_ui->m_selectedTimeInput->time();
  time_duration posixTime(timeInput.hour(), timeInput.minute(),
    timeInput.second(), timeInput.msec());
  auto utcTime = ToUtcTime(posixTime);
  return utcTime;
}

void TimeInputDialog::OnCurrentTimer() {
  auto currentTime = ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime());
  m_ui->m_currentTimeDisplay->setTime(PosixTimeToQTime(
    currentTime.time_of_day()));
}
