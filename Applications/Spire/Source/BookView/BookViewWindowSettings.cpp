#include "Spire/BookView/BookViewWindowSettings.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_BookViewPanel.h"
#include "ui_BookViewWindow.h"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Nexus;
using namespace Spire;
using namespace std;

BookViewWindowSettings::BookViewWindowSettings() {}

BookViewWindowSettings::BookViewWindowSettings(const BookViewWindow& window,
    Ref<UserProfile> userProfile)
    : m_properties(window.GetBookViewProperties()),
      m_security(window.m_security),
      m_securityViewStack(window.m_securityViewStack),
      m_identifier(window.GetIdentifier()),
      m_linkIdentifier(window.m_linkIdentifier),
      m_geometry(window.saveGeometry()),
      m_bidPanelHeader(window.m_ui->m_bidPanel->m_ui->m_bookView->
        horizontalHeader()->saveState()),
      m_askPanelHeader(window.m_ui->m_askPanel->m_ui->m_bookView->
        horizontalHeader()->saveState()) {
  if(m_security != Security()) {
    m_name = "Book View - " +
      ToString(m_security, userProfile->GetMarketDatabase());
  } else {
    m_name = "Book View";
  }
}

string BookViewWindowSettings::GetName() const {
  return m_name;
}

QWidget* BookViewWindowSettings::Reopen(
    Ref<UserProfile> userProfile) const {
  BookViewWindow* window = new BookViewWindow(Ref(userProfile), m_properties,
    m_identifier);
  window->setAttribute(Qt::WA_DeleteOnClose);
  Apply(Ref(userProfile), Store(*window));
  return window;
}

void BookViewWindowSettings::Apply(Ref<UserProfile> userProfile,
    Out<QWidget> widget) const {
  BookViewWindow& window = dynamic_cast<BookViewWindow&>(*widget);
  window.restoreGeometry(m_geometry);
  window.m_ui->m_bidPanel->m_ui->m_bookView->horizontalHeader()->
    restoreState(m_bidPanelHeader);
  window.m_ui->m_askPanel->m_ui->m_bookView->horizontalHeader()->
    restoreState(m_askPanelHeader);
  window.m_securityViewStack = m_securityViewStack;
  if(m_security != Security()) {
    window.DisplaySecurity(m_security);
  }
  window.m_linkIdentifier = m_linkIdentifier;
}
