#include "Spire/InputWidgets/DateTimeInputDialog.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "ui_DateTimeInputDialog.h"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 1000;
}

DateTimeInputDialog::DateTimeInputDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_DateTimeInputDialog>()),
      m_userProfile(userProfile.get()) {
  m_ui->setupUi(this);
  m_ui->m_currentTimeDisplay->setTime(ToQTime(to_local_time(
    m_userProfile->GetClients().get_time_client().get_time()).
    time_of_day()));
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &DateTimeInputDialog::accept);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &DateTimeInputDialog::reject);
  connect(&m_currentTimer, &QTimer::timeout, this,
    &DateTimeInputDialog::OnCurrentTimer);
  m_currentTimer.start(UPDATE_INTERVAL);
}

DateTimeInputDialog::DateTimeInputDialog(const ptime& initialValue,
    Ref<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_DateTimeInputDialog>()),
      m_userProfile(userProfile.get()) {
  m_ui->setupUi(this);
  m_ui->m_currentTimeDisplay->setTime(ToQTime(to_local_time(
    m_userProfile->GetClients().get_time_client().get_time()).
    time_of_day()));
  if(!initialValue.is_special() && !initialValue.is_not_a_date_time()) {
    ptime localInitialvalue = to_local_time(initialValue);
    date initialDate = localInitialvalue.date();
    if(!initialDate.is_special() && !initialDate.is_not_a_date()) {
      QDate displayDate(initialDate.year(), initialDate.month(),
        initialDate.day());
      m_ui->m_calendar->setSelectedDate(displayDate);
    }
    time_duration timeOfDay = localInitialvalue.time_of_day();
    if(!timeOfDay.is_special() && !timeOfDay.is_not_a_date_time()) {
      m_ui->m_selectedTimeInput->setTime(ToQTime(timeOfDay));
    }
  }
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &DateTimeInputDialog::accept);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &DateTimeInputDialog::reject);
  connect(&m_currentTimer, &QTimer::timeout, this,
    &DateTimeInputDialog::OnCurrentTimer);
  m_currentTimer.start(UPDATE_INTERVAL);
}

DateTimeInputDialog::~DateTimeInputDialog() {}

ptime DateTimeInputDialog::GetDateTime() const {
  QDate dateInput = m_ui->m_calendar->selectedDate();
  QTime timeInput = m_ui->m_selectedTimeInput->time();
  QDateTime dateTimeInput(dateInput, timeInput);
  QDateTime utcDateTime = dateTimeInput.toUTC();
  ptime posixTime = ToPosixTime(utcDateTime);
  return posixTime;
}

void DateTimeInputDialog::OnCurrentTimer() {
  m_ui->m_currentTimeDisplay->setTime(ToQTime(to_local_time(
    m_userProfile->GetClients().get_time_client().get_time()).time_of_day()));
}
