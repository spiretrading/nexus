#include "Spire/UI/LoginDialog.hpp"
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/ServiceLocator/SessionEncryption.hpp>
#include <boost/lexical_cast.hpp>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include "ui_LoginDialog.h"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const QString FOCUS_IN_STYLE = "\
    QWidget#m_usernameBox {\
      background-color: rgb(174, 203, 232);\
    }\
    QWidget#m_passwordBox {\
      background-color: rgb(174, 203, 232);\
    }\
    QWidget#m_serverBox {\
      background-color: rgb(174, 203, 232);\
    }\
  ";

  int UPDATE_INTERVAL = 100;
}

struct LoginDialog::LoginRoutine {
  int m_loginCount;
  unique_ptr<ApplicationServiceLocatorClient> m_serviceLocatorClient;
  string m_error;
  RoutineTaskQueue m_routines;

  LoginRoutine();
};

LoginDialog::LoginRoutine::LoginRoutine()
    : m_loginCount(0) {}

LoginDialog::LoginDialog(std::vector<ServerEntry> servers)
    : m_ui(std::make_unique<Ui_LoginDialog>()),
      m_servers(std::move(servers)),
      m_loginRoutine(std::make_unique<Sync<LoginRoutine>>()),
      m_state(READY) {
  m_ui->setupUi(this);
  for(auto& server : m_servers) {
    m_ui->m_serverComboBox->addItem(QString::fromStdString(server.m_name));
  }
  auto desktopGeometry = QGuiApplication::primaryScreen()->geometry();
  move(desktopGeometry.center() - rect().center());
  m_ui->m_usernameInput->installEventFilter(this);
  m_ui->m_passwordInput->installEventFilter(this);
  m_ui->m_serverComboBox->installEventFilter(this);
  m_ui->m_usernameInput->setFocus();
  connect(m_ui->m_passwordInput, &QLineEdit::textChanged, this,
    &LoginDialog::OnPasswordTextChanged);
  connect(m_ui->m_loginButton, &QPushButton::clicked, this,
    &LoginDialog::OnLoginButtonClicked);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &LoginDialog::OnCancelButtonClicked);
  connect(&m_updateTimer, &QTimer::timeout, this, &LoginDialog::OnUpdateTimer);
  UpdateChromaHash();
  m_updateTimer.start(UPDATE_INTERVAL);
  setWindowTitle("Login - Version " SPIRE_VERSION);
}

LoginDialog::~LoginDialog() {
  With(*m_loginRoutine,
    [&] (LoginRoutine& loginRoutine) {
      ++loginRoutine.m_loginCount;
    });
}

unique_ptr<ApplicationServiceLocatorClient>
    LoginDialog::GetServiceLocatorClient() {
  return std::move(m_serviceLocatorClient);
}

string LoginDialog::GetUsername() const {
  return m_ui->m_usernameInput->text().toStdString();
}

string LoginDialog::GetPassword() const {
  return m_ui->m_passwordInput->text().toStdString();
}

bool LoginDialog::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_usernameInput && event->type() == QEvent::FocusIn) {
    m_ui->m_usernameBox->setStyleSheet(FOCUS_IN_STYLE);
  } else if(object == m_ui->m_usernameInput &&
      event->type() == QEvent::FocusOut) {
    m_ui->m_usernameBox->setStyleSheet("");
  } else if(object == m_ui->m_passwordInput &&
      event->type() == QEvent::FocusIn) {
    m_ui->m_passwordBox->setStyleSheet(FOCUS_IN_STYLE);
  } else if(object == m_ui->m_passwordInput &&
      event->type() == QEvent::FocusOut) {
    m_ui->m_passwordBox->setStyleSheet("");
  } else if(object == m_ui->m_serverComboBox &&
      event->type() == QEvent::FocusIn) {
    m_ui->m_serverBox->setStyleSheet(FOCUS_IN_STYLE);
  } else if(object == m_ui->m_serverComboBox &&
      event->type() == QEvent::FocusOut) {
    m_ui->m_serverBox->setStyleSheet("");
  }
  return QWidget::eventFilter(object, event);
}

void LoginDialog::UpdateChromaHash() {
  static const auto CHROMA_HASHES = 3;
  static const auto COLOR_LENGTH = 6;
  auto password = m_ui->m_passwordInput->text().toStdString();
  auto hash = ComputeSHA(password);
  for(auto i = 0; i < CHROMA_HASHES; ++i) {
    auto color = hash.substr(COLOR_LENGTH * i, COLOR_LENGTH);
    auto styleSheet = QString::fromStdString(
    "\
      QWidget#m_chromaHash" + lexical_cast<string>(i + 1) + " {\
        background-color: #" + color + ";\
      }\
    ");
    if(i == 0) {
      m_ui->m_chromaHash1->setStyleSheet(styleSheet);
    } else if(i == 1) {
      m_ui->m_chromaHash2->setStyleSheet(styleSheet);
    } else {
      m_ui->m_chromaHash3->setStyleSheet(styleSheet);
    }
  }
}

void LoginDialog::OnPasswordTextChanged(const QString& text) {
  UpdateChromaHash();
}

void LoginDialog::OnLoginButtonClicked() {
  m_ui->m_resultLabel->clear();
  m_ui->m_resultLabel->repaint();
  auto address = m_servers[m_ui->m_serverComboBox->currentIndex()].m_address;
  auto username = GetUsername();
  auto password = GetPassword();
  int currentLoginCount;
  m_state = LOADING;
  With(*m_loginRoutine,
    [&] (LoginRoutine& loginRoutine) {
      ++loginRoutine.m_loginCount;
      currentLoginCount = loginRoutine.m_loginCount;
    });
  auto routine =
    [=] {
      auto serviceLocatorClient =
        std::make_unique<ApplicationServiceLocatorClient>();
      try {
        serviceLocatorClient->BuildSession(username, password, address);
        With(*m_loginRoutine,
          [=, &serviceLocatorClient] (LoginRoutine& loginRoutine) {
            if(loginRoutine.m_loginCount != currentLoginCount) {
              return;
            }
            loginRoutine.m_serviceLocatorClient.swap(serviceLocatorClient);
          });
      } catch(const AuthenticationException&) {
        With(*m_loginRoutine,
          [=] (LoginRoutine& loginRoutine) {
            if(loginRoutine.m_loginCount != currentLoginCount) {
              return;
            }
            loginRoutine.m_error = "Invalid username or password.";
          });
      } catch(const std::exception&) {
        With(*m_loginRoutine,
          [=] (LoginRoutine& loginRoutine) {
            if(loginRoutine.m_loginCount != currentLoginCount) {
              return;
            }
            loginRoutine.m_error = "Login connection failed.";
          });
      }
    };
  m_ui->m_usernameInput->setEnabled(false);
  m_ui->m_passwordInput->setEnabled(false);
  m_ui->m_serverComboBox->setEnabled(false);
  m_ui->m_loginButton->setEnabled(false);
  With(*m_loginRoutine,
    [&] (LoginRoutine& loginRoutine) {
      loginRoutine.m_routines.Push(routine);
    });
}

void LoginDialog::OnCancelButtonClicked() {
  if(m_state == READY) {
    Q_EMIT reject();
  } else {
    With(*m_loginRoutine,
      [&] (LoginRoutine& loginRoutine) {
        ++loginRoutine.m_loginCount;
        loginRoutine.m_serviceLocatorClient.reset();
        loginRoutine.m_error.clear();
      });
    m_state = READY;
    m_ui->m_usernameInput->setEnabled(true);
    m_ui->m_passwordInput->setEnabled(true);
    m_ui->m_serverComboBox->setEnabled(true);
    m_ui->m_loginButton->setEnabled(true);
    m_ui->m_resultLabel->setText(tr("Login canceled by user."));
  }
}

void LoginDialog::OnUpdateTimer() {
  if(m_state != LOADING) {
    return;
  }
  string error;
  With(*m_loginRoutine,
    [&] (LoginRoutine& loginRoutine) {
      m_serviceLocatorClient.swap(loginRoutine.m_serviceLocatorClient);
      error.swap(loginRoutine.m_error);
    });
  if(m_serviceLocatorClient == nullptr && error.empty()) {
    return;
  }
  m_ui->m_usernameInput->setEnabled(true);
  m_ui->m_passwordInput->setEnabled(true);
  m_ui->m_serverComboBox->setEnabled(true);
  m_ui->m_loginButton->setEnabled(true);
  m_state = READY;
  if(m_serviceLocatorClient != nullptr) {
    Q_EMIT accept();
  }
  if(!error.empty()) {
    m_ui->m_resultLabel->setText(QString::fromStdString(error));
  }
}
