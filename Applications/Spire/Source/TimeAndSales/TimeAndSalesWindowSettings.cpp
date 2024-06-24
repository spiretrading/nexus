#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
#include <QScreen>
#include <QWindow>
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "ui_TimeAndSalesWindow.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  void restore(QWidget& widget, const QByteArray& geometry) {
    if(geometry.size() < 4) {
      return;
    }
    QDataStream stream(geometry);
    stream.setVersion(QDataStream::Qt_4_0);
    const quint32 magicNumber = 0x1D9D0CB;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if(storedMagicNumber != magicNumber) {
        return;
    }
    const quint16 currentMajorVersion = 3;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;
    stream >> majorVersion >> minorVersion;
    if(majorVersion > currentMajorVersion) {
      return;
    }
    QRect restoredFrameGeometry;
    QRect restoredGeometry;
    QRect restoredNormalGeometry;
    qint32 restoredScreenNumber;
    quint8 maximized;
    quint8 fullScreen;
    qint32 restoredScreenWidth = 0;
    stream >> restoredFrameGeometry
           >> restoredNormalGeometry
           >> restoredScreenNumber
           >> maximized
           >> fullScreen;
    if(majorVersion > 1) {
      stream >> restoredScreenWidth;
    }
    if(majorVersion > 2) {
      stream >> restoredGeometry;
    }
    if(restoredScreenNumber >= QApplication::screens().count()) {
      restoredScreenNumber = QApplication::screens().indexOf(
        QApplication::primaryScreen());
    }
    auto restoredScreen = QApplication::screens()[restoredScreenNumber];
    const qreal screenWidthF = qreal(restoredScreen->geometry().width());
    const qreal width_factor = screenWidthF / qreal(restoredScreenWidth);
    const int frameHeight = 20;
    const QRect availableGeometry = restoredScreen->availableGeometry();
    widget.setWindowState(
      widget.windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen));
    auto new_geometry = [&] {
      if(majorVersion > 2) {
        return restoredGeometry;
      } else {
        return restoredNormalGeometry;
      }
    }();
    if(width_factor != 1.0) {
      new_geometry = QRect(new_geometry.x() * width_factor,
        new_geometry.y() * width_factor, new_geometry.width() * width_factor,
        new_geometry.height() * width_factor);
    }
    widget.setGeometry(new_geometry);
  }
}

TimeAndSalesWindowSettings::TimeAndSalesWindowSettings() {}

TimeAndSalesWindowSettings::TimeAndSalesWindowSettings(
    const TimeAndSalesWindow& window, Ref<UserProfile> userProfile)
    : m_properties(window.GetProperties()),
      m_security(window.m_security),
      m_securityViewStack(window.m_securityViewStack),
      m_identifier(window.GetIdentifier()),
      m_linkIdentifier(window.m_linkIdentifier),
      m_geometry(window.saveGeometry()),
      m_splitterState(window.m_ui->m_splitter->saveState()),
      m_viewHeaderState(
        window.m_ui->m_timeAndSalesView->horizontalHeader()->saveState()),
      m_snapshotHeaderState(
        window.m_ui->m_snapshotView->horizontalHeader()->saveState()) {
  if(m_security == Security()) {
    m_name = "Time And Sales";
  } else {
    m_name = "Time And Sales - " +
      ToString(m_security, userProfile->GetMarketDatabase());
  }
}

TimeAndSalesWindowSettings::~TimeAndSalesWindowSettings() {}

string TimeAndSalesWindowSettings::GetName() const {
  return m_name;
}

QWidget* TimeAndSalesWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  TimeAndSalesWindow* window = new TimeAndSalesWindow(Ref(userProfile),
    m_properties, m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), Store(*window));
  return window;
}

void TimeAndSalesWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  TimeAndSalesWindow& window = dynamic_cast<TimeAndSalesWindow&>(*widget);
  restore(window, m_geometry);
//  window.restoreGeometry(m_geometry);
  window.m_ui->m_splitter->restoreState(m_splitterState);
  window.m_ui->m_timeAndSalesView->horizontalHeader()->restoreState(
    m_viewHeaderState);
  window.m_ui->m_snapshotView->horizontalHeader()->restoreState(
    m_snapshotHeaderState);
  window.m_securityViewStack = m_securityViewStack;
  if(m_security != Security()) {
    window.DisplaySecurity(m_security);
  }
  window.m_linkIdentifier = m_linkIdentifier;
}
